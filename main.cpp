#include "raylib.h"
#include <vector>
#include <cstring>
#include <string>
#include "cmath"
#include <cstdio>
#include <cstdlib>


//TEXTURESSSS
Texture2D playerTexture;
Texture2D floorTexture;
Texture2D backgroundTexture;
Texture2D boxTexture;
Texture2D spikeTexture;

// === COLOR SCHEME 2.0 ===
// Spikes & Obstacles
Color spikeTint        = { 255, 80, 140, 240 };  // Neon magenta (danger pop)
Color boxTint          = { 80, 200, 255, 240 };  // Sky neon cyan

// Floors
Color floorUpTint      = { 255, 180, 80, 200 };  // Warm orange floor
Color floorDownTint    = { 100, 220, 180, 200 }; // Teal-green floor

// Player
Color playerUpTint     = { 255, 255, 120, 255 }; // Bright gold (hero glow)
Color playerDownTint   = { 150, 255, 200, 255 }; // Soft minty glow

// Backgrounds
Color bgUpTint         = { 120, 40, 200, 255 };    // Cosmic indigo haze
Color bgDownTint       = { 40, 80, 200, 255 };    // Deep navy night

// Particles (generic sparks)
Color particleUpTint   = { 255, 120, 200, 255 }; // Pink sparks
Color particleDownTint = { 120, 220, 255, 255 }; // Cyan sparks

// Dust
Color dustJumpTint     = { 230, 200, 150, 255 }; // Sandy beige dust
Color dustLandTint     = { 190, 190, 190, 255 }; // Light gray dust

// Extra sparks
Color sparkUpTint      = { 255, 200, 100, 255 }; // Ember orange sparks
Color sparkDownTint    = { 100, 180, 255, 255 }; // Electric blue sparks

// Trail
Color trailTintUp      = { 255, 150, 80, 220 };  // Warm neon orange trail
Color trailTintDown    = { 100, 255, 200, 220 }; // Aqua-mint trail




//Constants i suppose

bool wasGroundedLastFrame = true;
float shakeDuration = 0.0f;
const float shakeMagnitude = 2.0f;
const float gravity = 0.8f;
const float jumpForce = 12.0f;
const int groundY = 640;
const int ceilingY = 80;
const Vector2 restartPosition = { 400, groundY - 40 }; // Player start position
const float scrollSpeed = 4.0f;

Vector2 shakeOffset = { 0, 0 };

//Screen dimensions

const int screenWidth = 1280;
const int screenHeight =720;
const int TILE_SIZE = 40;


std::vector<std::string> level;

//Dis is just initializin for the loadlevel func
int levelRows;
int levelCols;

//Game states 
enum GameState { START, PLAYING, GAMEOVER };
GameState gameState = START;

class Particle {
public:
    Vector2 position;
    Vector2 velocity;
    float size;
    float lifetime;     // remaining
    float maxLifetime;  // initial
    Color color;
    bool additive;      // draw in additive pass?

    Particle(Vector2 pos, Vector2 vel, float s, float life, Color col, bool add = false)
        : position(pos), velocity(vel), size(s), lifetime(life), maxLifetime(life), color(col), additive(add) {}

    void Update(float dt) {
        position.x += velocity.x * dt;
        position.y += velocity.y * dt;
        lifetime   -= dt;
    }

    void Draw() const {
        float t = (maxLifetime > 0.0f) ? (lifetime / maxLifetime) : 0.0f; // 1 -> 0
        if (t < 0.0f) t = 0.0f; 
        if (t > 1.0f) t = 1.0f;
        Color c = color;
        c.a = (unsigned char)(color.a * t);   // fade out
        float r = size * (0.5f + 0.5f * t);   // shrink slightly
        DrawCircleV(position, r, c);
    }

    bool IsAlive() const { return lifetime > 0.0f; }
    bool IsDead()  const { return lifetime <= 0.0f; }
};

std::vector<Particle> particles;


class Player{
public:
    Vector2 pos;

    int gravityDirection = 1;

    float width = TILE_SIZE - 10, height = TILE_SIZE - 10;
    float velocityX = 0 , velocityY= 0;
    float rotation = 0.0f;
    float targetRotation = 0.0f;
    float rotationSpeed = 6.0f;

    bool rotating = false;
    bool isGrounded = false;

    Player(float x, float y){
        pos = { x,y };
    }

    void Update(){
        if (pos.y + height / 2 < screenHeight / 2) {
            gravityDirection = -1;
        } else {
            gravityDirection = 1;
        }

        velocityY += gravity * gravityDirection;
        pos.y += velocityY;
        if (rotating) {
            if (fabs(rotation - targetRotation) > 0.1f) {
            float dir = (rotation < targetRotation) ? 1 : -1;
            rotation += dir * rotationSpeed;
        } else {
            rotation = targetRotation;
            rotating = false;
        }
        }

    }

    void Jump() {
        if (isGrounded) {
            velocityY = -jumpForce * gravityDirection;
            targetRotation += 90.0f;
            rotating = true;
            isGrounded = false;

            // --- Emit jump dust + a few bright sparks from the foot ---
            Vector2 foot = { pos.x + width * 0.5f, (gravityDirection == 1) ? (pos.y + height) : pos.y };

            // Dust (normal blending)
            for (int i = 0; i < 12; ++i) {
                float baseDeg   = (gravityDirection == 1) ? -90.0f : 90.0f;   // away from floor
                float spreadDeg = (float)GetRandomValue(-70, 70);
                float ang       = (baseDeg + spreadDeg) * DEG2RAD;
                float speed     = 140.0f + (float)GetRandomValue(0, 70);
                Vector2 vel     = { cosf(ang) * speed, sinf(ang) * speed };

                float sz   = 4.0f + (float)GetRandomValue(0, 15) * 0.1f;
                float life = 0.2f + (float)GetRandomValue(0, 8) * 0.01f;
                particles.push_back(Particle(foot, vel, sz, life, dustJumpTint, false));
            }

            // A few sparks (additive)
            Color sparkTint = (gravityDirection == 1) ? sparkUpTint : sparkDownTint;
            for (int i = 0; i < 5; ++i) {
                float baseDeg   = (gravityDirection == 1) ? -90.0f : 90.0f;
                float spreadDeg = (float)GetRandomValue(-40, 40);
                float ang       = (baseDeg + spreadDeg) * DEG2RAD;
                float speed     = 220.0f + (float)GetRandomValue(0, 120);
                Vector2 vel     = { cosf(ang) * speed, sinf(ang) * speed };

                float sz   = 4.0f + (float)GetRandomValue(0, 8) * 0.1f;
                float life = 0.12f + (float)GetRandomValue(0, 10) * 0.01f;
                particles.push_back(Particle(foot, vel, sz, life, sparkTint, true));
            }
        }
    }




    void Draw() {
        Vector2 origin = { width / 2, height / 2 };
        Vector2 center = { pos.x + width / 2, pos.y + height / 2 };
        Color tint = (pos.y + height / 2 < GetScreenHeight() / 2) ? playerDownTint : playerUpTint;
        DrawTexturePro(playerTexture,{0, 0, (float)playerTexture.width, (float)playerTexture.height},{ center.x, center.y, width, height }, origin, rotation, tint);
    }


    Rectangle GetRect(){
        return { pos.x, pos.y, width, height};
    }

};

class Spike{
public:
    Vector2 pos;
    float size = TILE_SIZE;

    Spike(float x, float y){
        pos = { x,y };
    }

    void Update(float speed){
        pos.x -= speed;
    }

    void Draw() {
        Rectangle src = {0, 0, (float)spikeTexture.width, (float)spikeTexture.height};
        Rectangle dst = GetRect();
        if (dst.y < screenHeight / 2) src.height = -src.height;
        DrawTexturePro(spikeTexture, src, dst, {0,0}, 0, spikeTint);
    }

    Rectangle GetRect(){
        return { pos.x, pos.y, size, size };
    }
};

class Box {
public:
    Rectangle rect;
    Box(float x, float y) {
        rect = { x, y, TILE_SIZE, TILE_SIZE };
    }

    void Draw() {
        Rectangle src = {0, 0, (float)boxTexture.width, (float)boxTexture.height};
        Rectangle dst = GetRect();
        if (dst.y < screenHeight / 2) src.height = -src.height;
        DrawTexturePro(boxTexture, src, dst, {0, 0}, 0, boxTint);
    }

    Rectangle GetRect() {
        return rect;
    }
};



std::vector<Rectangle> groundTiles;
std::vector<Spike> spikes;
std::vector<Box> boxes;
std::vector<Vector2> trailPositions;

Player p1(restartPosition.x, restartPosition.y);

Camera2D camera = { 0 };

void LoadLevelFromFile(const char* filename) {
    level.clear();
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        TraceLog(LOG_ERROR, "Could not open level file!");
        return;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = '\0';
        level.push_back(std::string(buffer));
    }

    fclose(file);

    // Update level dimensions
    levelRows = level.size();
    levelCols = level.empty() ? 0 : level[0].size();

}


void LoadLevel() {
    for (int y = 0; y < levelRows; y++) {
        for (int x = 0; x < levelCols; x++) {
            char tile = level[y][x];
            float worldX = x * TILE_SIZE;
            float worldY = y * TILE_SIZE;

            if (tile == '#') {
                groundTiles.push_back({ worldX, worldY, TILE_SIZE, TILE_SIZE });
            }
            else if (tile == '^') {
                spikes.push_back(Spike(worldX, worldY));
            }
            else if (tile == 'B') {
                boxes.push_back(Box(worldX, worldY));
            }
        }
    }
}

void UpdateParticles() {
    // Update & cull
    for (int i = (int)particles.size() - 1; i >= 0; --i) {
        particles[i].Update(GetFrameTime());
        if (particles[i].IsDead()) {
            particles.erase(particles.begin() + i);
        }
    }

    // (optional) hard cap to avoid runaway allocations
    const size_t MAX_PARTICLES = 400;
    if (particles.size() > MAX_PARTICLES) {
        particles.erase(particles.begin(), particles.begin() + (particles.size() - MAX_PARTICLES));
    }
}

void ResetGame() {
    p1.pos = restartPosition;
    p1.velocityY = 0;
    p1.isGrounded = false;
    p1.rotation = 0.0f;
    p1.targetRotation = 0.0f;
    p1.rotating = false;

    // Reset map
    groundTiles.clear();
    spikes.clear();
    boxes.clear();
    particles.clear();
    trailPositions.clear();


    LoadLevelFromFile("level.txt");
    LoadLevel();


    // Reset game state

    gameState = START;
}

int main() {
    InitWindow(screenWidth, screenHeight, "Geometry Dash v2");
    SetTargetFPS(60);
    
    floorTexture = LoadTexture("assets/floor.png");
    playerTexture = LoadTexture("assets/player.png");
    boxTexture = LoadTexture("assets/box.png");
    spikeTexture = LoadTexture("assets/spike.png");
    backgroundTexture = LoadTexture("assets/background.png");

    LoadLevelFromFile("level.txt");
    LoadLevel();

    camera.target = {p1.pos.x + screenWidth/4, screenHeight/2.0f};
    camera.offset = {screenWidth/2.0f, screenHeight/2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        
        float scrollX = fmodf(p1.pos.x * 0.8f, backgroundTexture.width);
        if (scrollX < 0) scrollX += backgroundTexture.width; // ensure positive offset

        for (int x = -scrollX; x < screenWidth; x += backgroundTexture.width) {
            for (int y = 0; y < screenHeight; y += backgroundTexture.height) {
                DrawTexture(backgroundTexture, x, y, WHITE);
            }
        }



        DrawRectangleGradientV(
            0, 0,
            screenWidth, screenHeight,
            Fade(bgUpTint, 0.3f),   // Top tint
            Fade(bgDownTint, 0.3)     // Bottom tint
        );


        if(gameState == START){
            if(IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) gameState = PLAYING;
            DrawText("Press Enter to Start", screenWidth / 2 - 150, screenHeight / 2, 30, GRAY);
        }

        else if(gameState == PLAYING){
            if(IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) p1.Jump();
            
            p1.Update();
            p1.isGrounded = false;

            float prevX = p1.pos.x;
            float prevY = p1.pos.y;

            p1.pos.x += scrollSpeed;
            p1.velocityX = scrollSpeed; 

            camera.target.x = p1.pos.x + screenWidth/4;

            UpdateParticles();

            // Add current player position (center of player)
            trailPositions.push_back({ p1.pos.x + p1.width/2, (p1.gravityDirection == 1) ? p1.pos.y + p1.height - 8 : p1.pos.y + 8});

            // Limit how many points we store (otherwise vector grows forever)
            if (trailPositions.size() > 20) {
                trailPositions.erase(trailPositions.begin());
            }

            // Combined collision logic for ground and boxes
            std::vector<Rectangle> allRects = groundTiles;
            for (auto& b : boxes) allRects.push_back(b.GetRect());

            // Check for spike collision
            for (auto& s : spikes) {
                float spikePadding = 5.0f; 
                Rectangle spikeRect = s.GetRect();
                spikeRect.x += spikePadding;
                spikeRect.y += spikePadding;
                spikeRect.width -= spikePadding * 2;
                spikeRect.height -= spikePadding * 2;

                if (CheckCollisionRecs(p1.GetRect(), spikeRect)) {
                    gameState = GAMEOVER;
                }
            }

            for (auto& rect : allRects) {
                if (CheckCollisionRecs(p1.GetRect(), rect)) {

                    float px = p1.pos.x;
                    float py = p1.pos.y;
                    float pw = p1.width;
                    float ph = p1.height;

                    // Amount overlapped in both directions
                    float overlapX = std::min(px + pw, rect.x + rect.width) - std::max(px, rect.x);
                    float overlapY = std::min(py + ph, rect.y + rect.height) - std::max(py, rect.y);

                    // Vertical collision
                    if (overlapY < overlapX) {
                        // Hitting ground from above
                        if (p1.gravityDirection == 1 && p1.velocityY > 0) {
                            p1.pos.y = rect.y - ph;
                            p1.velocityY = 0;
                            p1.isGrounded = true;
                        }
                        // Hitting ceiling from below
                        else if (p1.gravityDirection == 1 && p1.velocityY < 0) {
                            p1.pos.y = rect.y + rect.height;
                            p1.velocityY = 0;
                        }
                        // Hitting ceiling in inverted gravity
                        else if (p1.gravityDirection == -1 && p1.velocityY < 0) {
                            p1.pos.y = rect.y + rect.height;
                            p1.velocityY = 0;
                            p1.isGrounded = true;
                        }
                        // Hitting ground in inverted gravity
                        else if (p1.gravityDirection == -1 && p1.velocityY > 0) {
                            p1.pos.y = rect.y - ph;
                            p1.velocityY = 0;
                        }
                    }
                    // Horizontal collision
                    else {
                        if (p1.velocityX > 0 && prevX + pw <= rect.x) {
                            p1.pos.x = rect.x - pw;
                            p1.velocityX = 0;
                        }
                        else if (p1.velocityX < 0 && prevX >= rect.x + rect.width) {
                            p1.pos.x = rect.x + rect.width;
                            p1.velocityX = 0;
                        }
                    }
                }
            }

            // Check for landing (was in air, now grounded)
            if (!wasGroundedLastFrame && p1.isGrounded) {
                shakeDuration = 0.2f;

                // Emit dust along the foot contact line
                Vector2 foot = { p1.pos.x + p1.width * 0.5f, (p1.gravityDirection == 1) ? (p1.pos.y + p1.height) : p1.pos.y };

                // Dust (normal)
                for (int i = 0; i < 16; ++i) {
                    float baseDeg   = (p1.gravityDirection == 1) ? 90.0f : -90.0f; // along the floor plane
                    float spreadDeg = (float)GetRandomValue(-100, 100);
                    float ang       = (baseDeg + spreadDeg) * DEG2RAD;
                    float speed     = 120.0f + (float)GetRandomValue(0, 80);
                    Vector2 vel     = { cosf(ang) * speed, sinf(ang) * speed };

                    float sz   = 4.0f + (float)GetRandomValue(0, 10) * 0.1f;
                    float life = 0.3f + (float)GetRandomValue(0, 10) * 0.01f;
                    particles.push_back(Particle(foot, vel, sz, life, dustLandTint, false));
                }

                // A couple of bright sparks (additive)
                Color sparkTint = (p1.gravityDirection == 1) ? sparkUpTint : sparkDownTint;
                for (int i = 0; i < 6; ++i) {
                    float baseDeg   = (p1.gravityDirection == 1) ? -90.0f : 90.0f; // away from floor
                    float spreadDeg = (float)GetRandomValue(-50, 50);
                    float ang       = (baseDeg + spreadDeg) * DEG2RAD;
                    float speed     = 200.0f + (float)GetRandomValue(0, 140);
                    Vector2 vel     = { cosf(ang) * speed, sinf(ang) * speed };

                    float sz   = 4.0f + (float)GetRandomValue(0, 8) * 0.1f;
                    float life = 0.10f + (float)GetRandomValue(0, 8) * 0.01f;
                    particles.push_back(Particle(foot, vel, sz, life, sparkTint, true));
                }

            }

            wasGroundedLastFrame = p1.isGrounded;

            if (shakeDuration > 0) {
                shakeOffset.x = (float)(GetRandomValue(-100, 100)) / 100.0f * shakeMagnitude;
                shakeOffset.y = (float)(GetRandomValue(-100, 100)) / 100.0f * shakeMagnitude;
                shakeDuration -= GetFrameTime();
            } else {
                shakeOffset = { 0, 0 };
            }

            Camera2D shakenCamera = camera;
            shakenCamera.target.x += shakeOffset.x;
            shakenCamera.target.y += shakeOffset.y;

            BeginMode2D(shakenCamera);

            float worldY = screenHeight/2.0f;
            float worldLeft = camera.target.x - screenWidth;
            float worldRight = camera.target.x + screenWidth;
            
            // Draw trail before world so player sits on top of everything nicely
            int N = (int)trailPositions.size();
            for (int i = 0; i < N; ++i) {
                float t = (N > 1) ? (float)i / (N - 1) : 1.0f;     // 0..1 older->newer
                float radius = 2.0f + 6.0f * t;                    // thinner to thicker
                Color base = (p1.gravityDirection == 1) ? trailTintUp : trailTintDown;
                Color col  = Fade(base, (1.0f - t) * 0.5f); // older ones fade slower, newer ones more transparent                // fade younger points
                DrawCircleV(trailPositions[i], radius, col);
            }



            // 1) Draw ground first (background)
            for (auto& g : groundTiles) {
                float centerY = g.y + g.height / 2;
                Color tint = (centerY < GetScreenHeight() / 2) ? floorDownTint : floorUpTint;
                DrawTexturePro(floorTexture, { 0, 0, (float)floorTexture.width, (float)floorTexture.height }, g, { 0, 0 }, 0.0f, tint);
            }

            // 2) Draw boxes & spikes (still part of the world)
            for (auto& b : boxes)  b.Draw();
            for (auto& s : spikes) s.Draw();

            // 3) Draw particles ABOVE ground so you can see dust puffs
            // Normal-blended particles (dust)
            for (const auto& particle : particles) {
                if (!particle.additive) particle.Draw();
            }

            // Additive-blended particles (sparks)
            BeginBlendMode(BLEND_ADDITIVE);
            for (const auto& particle : particles) {
                if (particle.additive) particle.Draw();
            }
            EndBlendMode();

            // 4) Draw player last so they’re on top of the dust
            p1.Draw();
            
            EndMode2D();
        }

        else if (gameState == GAMEOVER){
            DrawText("Game Over",screenWidth / 2 - 100, screenHeight / 2 - 30, 40, RED);
            DrawText("Press R to Restart", screenWidth / 2 - 160, screenHeight / 2 + 30, 25, DARKGRAY);

            //Reset game States
            if (IsKeyPressed(KEY_R) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ResetGame();

        }   

        EndDrawing();
    }

    //UNLOAD DA TAXTURES
    UnloadTexture(playerTexture);
    UnloadTexture(floorTexture);
    UnloadTexture(backgroundTexture);
    UnloadTexture(boxTexture);
    UnloadTexture(spikeTexture);

    CloseWindow();
    return 0;
}

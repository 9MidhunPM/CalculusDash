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

// === COLOR SCHEME ===
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
float gameOverTimer = 0.0f;
const float gameOverDelay = 2.0f; 
const float shakeMagnitude = 2.0f;
const float gravity = 0.8f;
const float jumpForce = 12.0f;
const int groundY = 640;
const int ceilingY = 80;
const Vector2 restartPosition = { 400, groundY - 40 };
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
enum GameState { START, PLAYING, GAMEOVER, VICTORY };
GameState gameState = START;

class Particle {
public:
    Vector2 position;
    Vector2 velocity;
    float size;
    float lifetime;     // remaining
    float maxLifetime;  // initial
    Color color;
    bool additive; 

    Particle(Vector2 pos, Vector2 vel, float s, float life, Color col, bool add = false)
        : position(pos), velocity(vel), size(s), lifetime(life), maxLifetime(life), color(col), additive(add) {}

    void Update(float dt) {
        position.x += velocity.x * dt;
        position.y += velocity.y * dt;
        lifetime   -= dt;
    }

    void Draw() const {
        float t = (maxLifetime > 0.0f) ? (lifetime / maxLifetime) : 0.0f;
        if (t < 0.0f) t = 0.0f; 
        if (t > 1.0f) t = 1.0f;
        Color c = color;
        c.a = (unsigned char)(color.a * t);  
        float r = size * (0.5f + 0.5f * t); 
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

            Vector2 foot = { pos.x + width * 0.5f, (gravityDirection == 1) ? (pos.y + height) : pos.y };

            for (int i = 0; i < 12; ++i) {
                float baseDeg   = (gravityDirection == 1) ? -90.0f : 90.0f;
                float spreadDeg = (float)GetRandomValue(-70, 70);
                float ang       = (baseDeg + spreadDeg) * DEG2RAD;
                float speed     = 140.0f + (float)GetRandomValue(0, 70);
                Vector2 vel     = { cosf(ang) * speed, sinf(ang) * speed };

                float sz   = 4.0f + (float)GetRandomValue(0, 15) * 0.1f;
                float life = 0.2f + (float)GetRandomValue(0, 8) * 0.01f;
                particles.push_back(Particle(foot, vel, sz, life, dustJumpTint, false));
            }

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

struct MapText {
    std::string text;
    Vector2 pos;
    int fontSize;
    Color color;
};


std::vector<Rectangle> groundTiles;
std::vector<Spike> spikes;
std::vector<Box> boxes;
std::vector<Vector2> trailPositions;
std::vector<MapText> mapTexts = {
    {"Press SPACE or CLICK to jump", {400, 400}, 20, WHITE},
    {"Hit a Spike?... Well", {1400, 500}, 20, RED},
    {"Also Gravity is Flipped past About here", {2000, screenHeight/2}, 20, YELLOW},
    {"Maybe Jumping isnt always for the Best", {5555, 300}, 20, RED},
    {"FLIP TIMEEE!", {6300, screenHeight/2}, 20, YELLOW},
    {"Seems Simple? Good luck!", {7000, 500}, 20, RED},
    {"Hands Hurt Yet??", {9000, 400}, 20, WHITE},
    {"Yea theese ARE pretty hard good luck!", {8500, 500}, 20, RED},
    {"Timing is EVERYTHING", {11000, 200}, 20, RED},
    {"FLIP..FLIP..FLIP..FLIPPP", {10000, 500}, 20, YELLOW},
    {"AAAND UR DONE.. Good Job u WIN", {12000, 400}, 20, WHITE},
};

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
        buffer[strcspn(buffer, "\n")] = '\0';
        level.push_back(std::string(buffer));
    }

    fclose(file);

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
    for (int i = (int)particles.size() - 1; i >= 0; --i) {
        particles[i].Update(GetFrameTime());
        if (particles[i].IsDead()) {
            particles.erase(particles.begin() + i);
        }
    }

    const size_t MAX_PARTICLES = 400;
    if (particles.size() > MAX_PARTICLES) {
        particles.erase(particles.begin(), particles.begin() + (particles.size() - MAX_PARTICLES));
    }
}

void DrawMapTexts() {
    for (auto& mt : mapTexts) {
        DrawText(mt.text.c_str(), (int)mt.pos.x, (int)mt.pos.y, mt.fontSize, mt.color);
    }
}


void DrawWorld(){
    float scrollX = fmodf(p1.pos.x * 0.8f, backgroundTexture.width);
    if (scrollX < 0) scrollX += backgroundTexture.width; 

    for (int x = -scrollX; x < screenWidth; x += backgroundTexture.width) {
        for (int y = 0; y < screenHeight; y += backgroundTexture.height) {
            DrawTexture(backgroundTexture, x, y, WHITE);
        }
    }

    DrawRectangleGradientV(
        0, 0,
        screenWidth, screenHeight,
        Fade(bgUpTint, 0.3f),   
        Fade(bgDownTint, 0.3)  
    );    
}

void ResetGame() {
    p1.pos = restartPosition;
    p1.velocityY = 0;
    p1.isGrounded = false;
    p1.rotation = 0.0f;
    p1.targetRotation = 0.0f;
    p1.rotating = false;

    groundTiles.clear();
    spikes.clear();
    boxes.clear();
    particles.clear();
    trailPositions.clear();

    LoadLevelFromFile("level.txt");
    LoadLevel();

    gameState = START;
}

void DrawStartScreen() {
    ClearBackground((Color){20, 20, 30, 255});
    
    const char* title = "CALCULUS DASH";
    int fontSize = 80;
    int titleWidth = MeasureText(title, fontSize);
    DrawText(title, screenWidth/2 - titleWidth/2, screenHeight/4, fontSize, RAYWHITE);

    const char* subtitle = "Press ENTER / CLICK to Play";
    int subSize = 30;
    int subWidth = MeasureText(subtitle, subSize);
    DrawText(subtitle, screenWidth/2 - subWidth/2, screenHeight/2, subSize, GRAY);
}

void UpdateStartScreen() {
    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        gameState = PLAYING;
    }
}

void DrawGameOverScreen() {
        if (gameOverTimer < gameOverDelay) {
        BeginMode2D(camera);

        for (auto& g : groundTiles) {
            float centerY = g.y + g.height / 2;
            Color tint = (centerY < GetScreenHeight() / 2) ? floorDownTint : floorUpTint;
            DrawTexturePro(floorTexture, {0,0,(float)floorTexture.width,(float)floorTexture.height}, g, {0,0}, 0.0f, tint);
        }
        for (auto& b : boxes)  b.Draw();
        for (auto& s : spikes) s.Draw();

        for (const auto& particle : particles) if (!particle.additive) particle.Draw();
        BeginBlendMode(BLEND_ADDITIVE);
        for (const auto& particle : particles) if (particle.additive) particle.Draw();
        EndBlendMode();

        float flash = sinf(GetTime() * 20);
        if (flash > 0) p1.Draw();

        EndMode2D();

    } 
    else{
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
            (Color){50, 0, 0, 255},
            (Color){10, 0, 0, 255});

        // --- Pulsing GAME OVER text ---
        const char* title = "GAME OVER!";
        int fontSize = 80;

        float scale = 1.0f + 0.05f * sinf(GetTime() * 3.0f);
        int baseWidth = MeasureText(title, fontSize);
        int scaledWidth = (int)(baseWidth * scale);

        DrawText(title,
            screenWidth/2 - scaledWidth/2,
            screenHeight/3,
            (int)(fontSize * scale),
            RED);

        const char* retryText = "[R] Click To Retry";

        int retryWidth = MeasureText(retryText, 30);

        DrawText(retryText,
            screenWidth/2 - retryWidth/2,
            screenHeight/2 + 100,
            30, GRAY);
        }
    }

void DrawVictoryScreen(){
    DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
        (Color){255, 255, 0, 255},
        (Color){255, 255, 255, 255});

    // --- Pulsing Victory text ---
    const char* title = "VICTORYYY!";
    int fontSize = 80;

    float scale = 1.0f + 0.05f * sinf(GetTime() * 3.0f);
    int baseWidth = MeasureText(title, fontSize);
    int scaledWidth = (int)(baseWidth * scale);

    DrawText(title,
        screenWidth/2 - scaledWidth/2,
        screenHeight/3,
        (int)(fontSize * scale),
        RED);
    const char* victory1Text = "You Finished the Level! Your reward is Flex Points +1000";

    int victory1Width = MeasureText(victory1Text, 30);

    DrawText(victory1Text,
        screenWidth/2 - victory1Width/2,
        screenHeight/2 + 100,
        30, BLACK);    

    const char* victory2Text = "(send me ss i want opinions)";

    int victory2Width = MeasureText(victory2Text, 30);

    DrawText(victory2Text,
        screenWidth/2 - victory2Width/2,
        screenHeight/2 + 200,
        30, GRAY);   
}

void UpdateGameOverScreen() {
    gameOverTimer += GetFrameTime();

    if(gameOverTimer < gameOverDelay) {
        gameOverTimer += GetFrameTime();
        return;
    }

    if (IsKeyPressed(KEY_R)|| IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ResetGame();
        gameState = PLAYING;
    }
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

        if(gameState == START){
            DrawStartScreen();
            UpdateStartScreen();
        }

        else if(gameState == PLAYING){
            if(IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) p1.Jump();
            
            //DEVELOPMENT CHEATS MAKE SURE TO DELETE LATER U CHILD DONT U DARE FORGET MIDHUN
            if(IsKeyPressed(KEY_O)) (p1.gravityDirection == 1) ? p1.pos.y -= 300 : p1.pos.y += 300; // debug gravity flip
            if(IsKeyPressed(KEY_P)) p1.pos.x += 600; // debug forward

            if (p1.pos.x > 12500) gameState = VICTORY;
            
            DrawWorld();
            
            p1.Update();
            p1.isGrounded = false;

            float prevX = p1.pos.x;
            float prevY = p1.pos.y;

            p1.pos.x += scrollSpeed;
            p1.velocityX = scrollSpeed; 

            camera.target.x = p1.pos.x + screenWidth/4;

            UpdateParticles();

            trailPositions.push_back({ p1.pos.x + p1.width/2, (p1.gravityDirection == 1) ? p1.pos.y + p1.height - 8 : p1.pos.y + 8});

            if (trailPositions.size() > 20) {
                trailPositions.erase(trailPositions.begin());
            }

            std::vector<Rectangle> allRects = groundTiles;
            for (auto& b : boxes) allRects.push_back(b.GetRect());

            for (auto& s : spikes) {
                float spikePadding = 8.0f; 
                Rectangle spikeRect = s.GetRect();
                spikeRect.x += spikePadding;
                spikeRect.y += spikePadding;
                spikeRect.width -= spikePadding * 2;
                spikeRect.height -= spikePadding * 2;

                if (CheckCollisionRecs(p1.GetRect(), spikeRect)) {
                    gameState = GAMEOVER;
                    gameOverTimer = 0.0f;
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

                for (int i = 0; i < 16; ++i) {
                    float baseDeg   = (p1.gravityDirection == 1) ? 90.0f : -90.0f;
                    float spreadDeg = (float)GetRandomValue(-100, 100);
                    float ang       = (baseDeg + spreadDeg) * DEG2RAD;
                    float speed     = 120.0f + (float)GetRandomValue(0, 80);
                    Vector2 vel     = { cosf(ang) * speed, sinf(ang) * speed };

                    float sz   = 4.0f + (float)GetRandomValue(0, 10) * 0.1f;
                    float life = 0.3f + (float)GetRandomValue(0, 10) * 0.01f;
                    particles.push_back(Particle(foot, vel, sz, life, dustLandTint, false));
                }

                Color sparkTint = (p1.gravityDirection == 1) ? sparkUpTint : sparkDownTint;
                for (int i = 0; i < 6; ++i) {
                    float baseDeg   = (p1.gravityDirection == 1) ? -90.0f : 90.0f;
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
            
            int N = (int)trailPositions.size();
            for (int i = 0; i < N; ++i) {
                float t = (N > 1) ? (float)i / (N - 1) : 1.0f; 
                float radius = 2.0f + 6.0f * t; 
                Color base = (p1.gravityDirection == 1) ? trailTintUp : trailTintDown;
                Color col  = Fade(base, (1.0f - t) * 0.5f);
                DrawCircleV(trailPositions[i], radius, col);
            }

            DrawMapTexts();

            for (auto& g : groundTiles) {
                float centerY = g.y + g.height / 2;
                Color tint = (centerY < GetScreenHeight() / 2) ? floorDownTint : floorUpTint;
                DrawTexturePro(floorTexture, { 0, 0, (float)floorTexture.width, (float)floorTexture.height }, g, { 0, 0 }, 0.0f, tint);
            }

            for (auto& b : boxes)  b.Draw();
            for (auto& s : spikes) s.Draw();

            for (const auto& particle : particles) {
                if (!particle.additive) particle.Draw();
            }

            BeginBlendMode(BLEND_ADDITIVE);
            for (const auto& particle : particles) {
                if (particle.additive) particle.Draw();
            }
            EndBlendMode();

            p1.Draw();
            
            EndMode2D();
        }

        else if (gameState == GAMEOVER){
            DrawGameOverScreen();
            UpdateGameOverScreen();
        }
        else if (gameState == VICTORY){
            DrawVictoryScreen();
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
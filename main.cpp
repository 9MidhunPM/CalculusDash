#include "raylib.h"
#include <vector>
#include "string.h"
#include "cmath"
#include <cstdlib>

//Constants i suppose
const float gravity = 0.8f;
const float jumpForce = 12.0f;
const int groundY = 640;
const int ceilingY = 80;

//Screen dimensions

const int screenWidth = 1280;
const int screenHeight =720;
const int TILE_SIZE = 40;

const float scrollSpeed = 4.0f;


const char* level[] = {
    "##############################################################",
    "##############################################################",
    "........................................B.......................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    "................................................................",
    ".............................B..................................",
    "..........................B..B..................................",
    ".......................B..B..B..................................",
    "....................B..B..B..B..................................",
    ".................B..B..B..B..B..................................",
    ".............BB..B..B..B..B..B................................",
    "###########^^##^^#^^#^^#^^#^^#################################",
    "##############################################################"
};

const int levelRows = sizeof(level) / sizeof(level[0]);
const int levelCols = strlen(level[0]);

enum GameState { START, PLAYING, GAMEOVER };
GameState gameState = START;

class Player{
public:
    Vector2 pos;

    int gravityDirection = 1;

    float width = TILE_SIZE, height = TILE_SIZE;
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
            if (rotation < targetRotation) {
                rotation += rotationSpeed;
                if (rotation > targetRotation) rotation = targetRotation;
            } else {
                rotating = false;
            }
        }

    }

    void Jump(){
        if(isGrounded){
            velocityY = -jumpForce * gravityDirection;
            targetRotation += 90.0f;
            rotating = true;
        }
    }

    void Draw() {
    Vector2 origin = { width / 2, height / 2 };
    Vector2 center = { pos.x + width / 2, pos.y + height / 2 };
    DrawRectanglePro({ center.x, center.y, width, height }, origin, rotation, BLUE);
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
        DrawRectangleV(pos, { size,size }, RED);
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
        DrawRectangleRec(rect, BROWN);
    }

    Rectangle GetRect() {
        return rect;
    }
};

class Particle {
public:
    Vector2 position;
    Vector2 velocity;
    float size;
    float lifetime;
    float maxLifetime;
    Color color;

    Particle(Vector2 pos, Vector2 vel, float sz, float life, Color col) {
        position = pos;
        velocity = vel;
        size = sz;
        lifetime = life;
        maxLifetime = life;
        color = col;
    }

    void Update() {
        position.x += velocity.x;
        position.y += velocity.y;
        lifetime -= 0.5f/60.0f; // Assuming 60 FPS
    }

    void Draw() {
        Color fadeColor = color;
        fadeColor.a = (unsigned char)(255 * (lifetime / maxLifetime));
        DrawCircleV(position, size, fadeColor);
    }

    bool IsDead() {
        return lifetime <= 0;
    }
};

std::vector<Rectangle> groundTiles;
std::vector<Spike> spikes;
std::vector<Box> boxes;
std::vector<Particle> particles;

Player p1(100, screenHeight - TILE_SIZE * 3 );

Camera2D camera = { 0 };

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
    // Update existing particles
    for (int i = particles.size() - 1; i >= 0; i--) {
        particles[i].Update();
        if (particles[i].IsDead()) {
            particles.erase(particles.begin() + i);
        }
    }
    
    // Create new particles
    if (gameState == PLAYING) {
        Vector2 emitPos = { p1.pos.x + p1.width/2, p1.pos.y + p1.height/2 };
        
        // Emit 2 particles per frame
        for (int i = 0; i < 2; i++) {
            float randomAngle = (rand() % 360) * DEG2RAD;
            Vector2 vel = { 
                cosf(randomAngle) * 2.0f - scrollSpeed/2,
                sinf(randomAngle) * 2.0f 
            };
            float size = 1.0f + (rand() % 3);
            float lifetime = 0.5f + (rand() % 100) / 100.0f;
            
            Color particleColor = p1.gravityDirection == 1 ? BLUE : RED;
            particles.push_back(Particle(emitPos, vel, size, lifetime, particleColor));
        }
    }
}
void ResetGame() {
    p1.pos = { 100, screenHeight - TILE_SIZE * 3 };
    p1.velocityY = 0;
    p1.isGrounded = false;

    // Reset map
    groundTiles.clear();
    spikes.clear();
    boxes.clear();

    LoadLevel();

    // Reset game state

    gameState = START;
}

int main() {
    InitWindow(screenWidth, screenHeight, "Geometry Dash v2");
    SetTargetFPS(60);
    
    LoadLevel();

    camera.target = {p1.pos.x + screenWidth/4, screenHeight/2.0f};
    camera.offset = {screenWidth/2.0f, screenHeight/2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if(gameState == START){
            if(IsKeyPressed(KEY_ENTER)) gameState = PLAYING;
            DrawText("Press Enter to Start", screenWidth / 2 - 150, screenHeight / 2, 30, GRAY);
        }

        else if(gameState == PLAYING){
            if(IsKeyPressed(KEY_SPACE)) p1.Jump();
            
            p1.Update();
            p1.isGrounded = false;

            float prevX = p1.pos.x;
            float prevY = p1.pos.y;

            p1.pos.x += scrollSpeed;
            p1.velocityX = scrollSpeed; 

            camera.target.x = p1.pos.x + screenWidth/4;

            UpdateParticles();

            // Scroll world elements to the left
            //for (auto& g : groundTiles) g.x -= scrollSpeed;
            //for (auto& s : spikes) s.pos.x -= scrollSpeed;
            //for (auto& b : boxes)  b.rect.x -= scrollSpeed;

            // Combined collision logic for ground and boxes
            std::vector<Rectangle> allRects = groundTiles;
            for (auto& b : boxes) allRects.push_back(b.GetRect());

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



            // Check for spike collision
            for (auto& s : spikes) {
                if (CheckCollisionRecs(p1.GetRect(), s.GetRect())) {
                    gameState = GAMEOVER;
                }
            }

            BeginMode2D(camera);

             // Draw background color division
            float worldY = screenHeight/2.0f;
            float worldLeft = camera.target.x - screenWidth;
            float worldRight = camera.target.x + screenWidth;
            
            // Top half - blue tint
            DrawRectangle(worldLeft, 0, worldRight - worldLeft, worldY, ColorAlpha(BLUE, 0.1f));
            
            // Bottom half - red tint
            DrawRectangle(worldLeft, worldY, worldRight - worldLeft, screenHeight - worldY, ColorAlpha(RED, 0.1f));

            for(auto& particle : particles){
                particle.Draw();
            }

            p1.Draw();

            for (auto& g : groundTiles) DrawRectangleRec(g, DARKGRAY);
            for (auto& b : boxes) b.Draw();
            for (auto& s : spikes) s.Draw();
            
            EndMode2D();
        }

        else if (gameState == GAMEOVER){
            DrawText("Game Over",screenWidth / 2 - 100, screenHeight / 2 - 30, 40, RED);
            DrawText("Press R to Restart", screenWidth / 2 - 160, screenHeight / 2 + 30, 25, DARKGRAY);

            //Reset game States
            if (IsKeyPressed(KEY_R)) ResetGame();

        }   

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

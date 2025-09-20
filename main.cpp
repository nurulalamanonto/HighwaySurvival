#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

struct Car {
    float x, y;
    int type; // enemy car type
};

int main() {
    const int screenWidth = 400;
    const int screenHeight = 600;
    const int carSize = 40;
    InitWindow(screenWidth, screenHeight, "Subway Survival - Stay sharp, stay alive.");
    SetTargetFPS(60);

    srand(time(0));

    // ---- Load textures ----
    Texture2D playerCar = LoadTexture("mycar.png");
    Texture2D enemyCars[3];
    enemyCars[0] = LoadTexture("pickup.png");
    enemyCars[1] = LoadTexture("bus.png");
    enemyCars[2] = LoadTexture("police.png");

    // ---- Separate road and different roadside textures ----
    Texture2D roadTex = LoadTexture("roadtex.jpg");         // middle road
    Texture2D leftSideTex = LoadTexture("left.jpg");       // left side
    Texture2D rightSideTex = LoadTexture("right.jpg");     // right side

    // ---- Start menu background image ----
    Texture2D startBg = LoadTexture("start.png");       // Add your start menu image here

    // Game variables
    float playerX = screenWidth / 2 - carSize / 2;
    float playerY = screenHeight - carSize * 2;
    float playerSpeed = 6.0f;
    int lives = 1;
    int score = 0;
    int highScore = 0;   
    std::vector<Car> enemies;
    int frameCounter = 0;
    float enemySpeed = 3.0f;

    enum GameState { START, PLAYING, PAUSED, GAMEOVER };
    GameState state = START;

    int difficulty = 0;
    float roadOffset = 0;
    float roadSpeed = 4;

    while (!WindowShouldClose()) {
        // --------- Pause toggle ---------
        if (state == PLAYING && IsKeyPressed(KEY_P)) state = PAUSED;
        else if (state == PAUSED && IsKeyPressed(KEY_P)) state = PLAYING;

        if (state == START) {
            if (IsKeyPressed(KEY_ONE)) { difficulty = 0; playerSpeed = 6; enemySpeed = 3; state = PLAYING; }
            if (IsKeyPressed(KEY_TWO)) { difficulty = 1; playerSpeed = 7; enemySpeed = 5.5f; state = PLAYING; }
            if (IsKeyPressed(KEY_THREE)) { difficulty = 2; playerSpeed = 8; enemySpeed = 7; state = PLAYING; }

            BeginDrawing();
            ClearBackground(DARKGRAY);

            // Draw Start menu background
            DrawTexture(startBg, 0, 0, WHITE);

            DrawText("Subway Survival", screenWidth/2.2 - 100, 100, 30, WHITE);
            DrawText("Press 1 for EASY",   screenWidth/2 - 100, 250, 20, GREEN);
            DrawText("Press 2 for MEDIUM", screenWidth/2 - 100, 300, 20, BLUE);
            DrawText("Press 3 for HARD",   screenWidth/2 - 100, 350, 20, RED);
            EndDrawing();
        }

        else if (state == PLAYING && lives > 0) {
            frameCounter++;

            // ---- Road scrolling ----
            roadOffset += roadSpeed;
            if (roadOffset > roadTex.height) roadOffset = 0;

            // ---- Dynamic difficulty ----
            float scaledEnemySpeed = enemySpeed + (score / 50.0f);
            int spawnRate = 60 - (score / 15);
            if (spawnRate < 20) spawnRate = 20;

            // ---- Spawn enemies ----
            if (frameCounter % spawnRate == 0) {
                float lane = (rand() % 4) * 50 + 120;
                int type = rand() % 3;
                enemies.push_back({ lane, 0, type });
            }

            // ---- Move enemies ----
            for (auto &e : enemies) {
                e.y += scaledEnemySpeed;
                Texture2D enemyTex = enemyCars[e.type];
                if (e.y + enemyTex.height >= playerY &&
                    e.y <= playerY + playerCar.height &&
                    e.x < playerX + playerCar.width &&
                    e.x + enemyTex.width > playerX) {
                    lives--;
                    state = GAMEOVER;

                    // ---- Update High Score ----
                    if (score > highScore) {
                        highScore = score;
                    }
                }
            }

            enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                        [&](Car &c){ return c.y > screenHeight; }), enemies.end());

            // ---- Player input ----
            if (IsKeyDown(KEY_LEFT) && playerX > 110) playerX -= playerSpeed;
            if (IsKeyDown(KEY_RIGHT) && playerX < 300 - playerCar.width) playerX += playerSpeed;

            if (frameCounter % 5 == 0) score++;

            // ---- Drawing ----
            BeginDrawing();
            ClearBackground(DARKGREEN);

            // ---- Draw left and right roadside textures ----
            for (int y = -leftSideTex.height; y < screenHeight; y += leftSideTex.height) {
                DrawTexture(leftSideTex, 0, y + (int)roadOffset, WHITE);                     
                DrawTexture(rightSideTex, screenWidth - rightSideTex.width, y + (int)roadOffset, WHITE); 
            }

            // ---- Draw road texture ----
            for (int y = -roadTex.height; y < screenHeight; y += roadTex.height) {
                DrawTexture(roadTex, (screenWidth - roadTex.width)/2, y + (int)roadOffset, WHITE);
            }

            // Player
            DrawTexture(playerCar, playerX, playerY, WHITE);

            // Enemies
            for (auto &e : enemies) {
                DrawTexture(enemyCars[e.type], e.x, e.y, WHITE);
            }

            // ---- Show score + high score ----
            DrawText(TextFormat("Score: %i", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Highest Score: %i", highScore), 10, 40, 20, YELLOW);
            EndDrawing();
        }

        else if (state == PAUSED) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("GAME PAUSED", screenWidth/2 - 120, screenHeight/2 - 40, 30, YELLOW);
            DrawText("Press P to Resume", screenWidth/2 - 110, screenHeight/2 + 10, 20, WHITE);
            DrawText("Press ESC to Quit", screenWidth/2 - 110, screenHeight/2 + 40, 20, WHITE);
            EndDrawing();
        }

        else if (state == GAMEOVER) {
            BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawText("GAME OVER!", screenWidth/2 - 100, screenHeight/2, 30, RED);
            DrawText(TextFormat("Final Score: %i", score), screenWidth/2 - 100, screenHeight/2 + 40, 20, WHITE);
            DrawText(TextFormat("Highest Score: %i", highScore), screenWidth/2 - 100, screenHeight/2 + 70, 20, YELLOW);
            DrawText("Press ENTER to Restart", screenWidth/2 - 120, screenHeight/2 + 100, 20, YELLOW);
            EndDrawing();

            if (IsKeyPressed(KEY_ENTER)) {
                lives = 1;
                score = 0;
                frameCounter = 0;
                enemies.clear();
                playerX = screenWidth / 2 - carSize / 2;
                roadOffset = 0;
                state = START;
            }
        }
    }

    // Unload textures
    UnloadTexture(playerCar);
    for (int i = 0; i < 3; i++) UnloadTexture(enemyCars[i]);
    UnloadTexture(roadTex);
    UnloadTexture(leftSideTex);
    UnloadTexture(rightSideTex);
    UnloadTexture(startBg);

    CloseWindow();
    return 0;
}

#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// 🟡 Helper function to draw text with border (outline)
void DrawTextWithBorder(const char* text, int posX, int posY, int fontSize, Color textColor, Color borderColor) {
    DrawText(text, posX - 1, posY, fontSize, borderColor);
    DrawText(text, posX + 1, posY, fontSize, borderColor);
    DrawText(text, posX, posY - 1, fontSize, borderColor);
    DrawText(text, posX, posY + 1, fontSize, borderColor);
    DrawText(text, posX - 1, posY - 1, fontSize, borderColor);
    DrawText(text, posX + 1, posY - 1, fontSize, borderColor);
    DrawText(text, posX - 1, posY + 1, fontSize, borderColor);
    DrawText(text, posX + 1, posY + 1, fontSize, borderColor);
    DrawText(text, posX, posY, fontSize, textColor);
}

struct Car {
    float x, y;
    int type; // enemy car type
};

int main() {
    const int screenWidth = 400;
    const int screenHeight = 600;
    const int carSize = 40;
    InitWindow(screenWidth, screenHeight, "Highway Survival - Stay sharp, stay alive.");
    SetTargetFPS(60);

    InitAudioDevice();
    srand(time(0));

    Texture2D playerCar = LoadTexture("Graphics/mycar.png");
    Texture2D enemyCars[4];   // 🔹 now 4 enemy types
    enemyCars[0] = LoadTexture("Graphics/pickup.png");
    enemyCars[1] = LoadTexture("Graphics/bus.png");
    enemyCars[2] = LoadTexture("Graphics/police.png");
    enemyCars[3] = LoadTexture("Graphics/taxi.png");   // 🟡 new enemy car

    Texture2D roadTex = LoadTexture("Graphics/roadtex.jpg");
    Texture2D leftSideTex = LoadTexture("Graphics/left.jpg");
    Texture2D rightSideTex = LoadTexture("Graphics/right.jpg");

    Texture2D startBg = LoadTexture("Graphics/start.png");       
    Texture2D gameOverBg = LoadTexture("Graphics/gameover.png");
    Texture2D pauseBg = LoadTexture("Graphics/start.png");

    Sound crashSound = LoadSound("Sounds/crash.ogg");
    Sound menuSelect = LoadSound("Sounds/select.ogg");
    Music bgMusic = LoadMusicStream("Sounds/bgMusic.ogg");
    SetSoundVolume(menuSelect, 5.0f);   
    SetSoundVolume(crashSound, 0.8f);    
    SetMusicVolume(bgMusic, 0.2f);

    float playerX = screenWidth / 2 - carSize / 2;
    float playerY = screenHeight - carSize * 2;
    float playerSpeed = 6.0f;
    int lives = 1;
    int score = 0;

    int highScoreEasy = 0;
    int highScoreMedium = 0;
    int highScoreHard = 0;

    std::vector<Car> enemies;
    int frameCounter = 0;
    float enemySpeed = 3.0f;

    enum GameState { START, PLAYING, PAUSED, GAMEOVER };
    GameState state = START;

    int difficulty = 0;
    float roadOffset = 0;
    float roadSpeed = 4;

    PlayMusicStream(bgMusic);

    while (!WindowShouldClose()) {
        UpdateMusicStream(bgMusic);

        if (state == PLAYING && IsKeyPressed(KEY_P)) state = PAUSED;
        else if (state == PAUSED && IsKeyPressed(KEY_P)) state = PLAYING;

        const char* difficultyName;
        switch (difficulty) {
            case 0: difficultyName = "EASY"; break;
            case 1: difficultyName = "MEDIUM"; break;
            case 2: difficultyName = "HARD"; break;
            default: difficultyName = "UNKNOWN"; break;
        }

        if (state == START) {
            if (IsKeyPressed(KEY_ONE)) { difficulty = 0; playerSpeed = 4; enemySpeed = 3; state = PLAYING; PlaySound(menuSelect); }
            if (IsKeyPressed(KEY_TWO)) { difficulty = 1; playerSpeed = 5; enemySpeed = 5.5f; state = PLAYING; PlaySound(menuSelect); }
            if (IsKeyPressed(KEY_THREE)) { difficulty = 2; playerSpeed = 6; enemySpeed = 7; state = PLAYING; PlaySound(menuSelect); }

            BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawTexture(startBg, 0, 0, WHITE);
            DrawTextWithBorder("Highway Survival", screenWidth/2 - MeasureText("Highway Survival", 30)/2, 100, 30, WHITE, BLACK);
            DrawTextWithBorder("Press 1 for EASY",   screenWidth/2 - MeasureText("Press 1 for EASY", 20)/2, 250, 20, GREEN, BLACK);
            DrawTextWithBorder("Press 2 for MEDIUM", screenWidth/2 - MeasureText("Press 2 for MEDIUM", 20)/2, 300, 20, YELLOW, BLACK);
            DrawTextWithBorder("Press 3 for HARD",   screenWidth/2 - MeasureText("Press 3 for HARD", 20)/2, 350, 20, RED, BLACK);
            EndDrawing();
        }

        else if (state == PLAYING && lives > 0) {
            frameCounter++;
            roadOffset += roadSpeed;
            if (roadOffset > roadTex.height) roadOffset = 0;

            float scaledEnemySpeed = enemySpeed + (score / 50.0f);
            int spawnRate = 60 - (score / 15);
            if (spawnRate < 20) spawnRate = 20;

            if (frameCounter % spawnRate == 0) {
                float lane = (rand() % 4) * 50 + 120;
                int type = rand() % 4;   // 🔹 include new car type
                enemies.push_back({ lane, 0, type });
            }

            for (auto &e : enemies) {
                e.y += scaledEnemySpeed;
                Texture2D enemyTex = enemyCars[e.type];
                if (e.y + enemyTex.height >= playerY &&
                    e.y <= playerY + playerCar.height &&
                    e.x < playerX + playerCar.width &&
                    e.x + enemyTex.width > playerX) {
                    lives--;
                    state = GAMEOVER;
                    PlaySound(crashSound);

                    if (difficulty == 0 && score > highScoreEasy) highScoreEasy = score;
                    if (difficulty == 1 && score > highScoreMedium) highScoreMedium = score;
                    if (difficulty == 2 && score > highScoreHard) highScoreHard = score;
                }
            }

            enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                        [&](Car &c){ return c.y > screenHeight; }), enemies.end());

            if (IsKeyDown(KEY_LEFT) && playerX > 110) playerX -= playerSpeed;
            if (IsKeyDown(KEY_RIGHT) && playerX < 300 - playerCar.width) playerX += playerSpeed;

            if (frameCounter % 5 == 0) score++;

            BeginDrawing();
            ClearBackground(DARKGREEN);

            for (int y = -leftSideTex.height; y < screenHeight; y += leftSideTex.height) {
                DrawTexture(leftSideTex, 0, y + (int)roadOffset, WHITE);                     
                DrawTexture(rightSideTex, screenWidth - rightSideTex.width, y + (int)roadOffset, WHITE); 
            }

            for (int y = -roadTex.height; y < screenHeight; y += roadTex.height) {
                DrawTexture(roadTex, (screenWidth - roadTex.width)/2, y + (int)roadOffset, WHITE);
            }

            DrawTexture(playerCar, playerX, playerY, WHITE);
            for (auto &e : enemies) DrawTexture(enemyCars[e.type], e.x, e.y, WHITE);

            int displayHighScore = 0;
            if (difficulty == 0) displayHighScore = highScoreEasy;
            if (difficulty == 1) displayHighScore = highScoreMedium;
            if (difficulty == 2) displayHighScore = highScoreHard;

            DrawTextWithBorder(TextFormat("Score: %i", score), 10, 10, 20, WHITE, BLACK);
            DrawTextWithBorder(TextFormat("Highest Score: %i (%s)", displayHighScore, difficultyName), 10, 40, 20, YELLOW, BLACK);
            EndDrawing();
        }

        else if (state == PAUSED) {
            BeginDrawing();
            DrawTexture(pauseBg, 0, 0, WHITE);
            const char* pauseText = "GAME PAUSED";
            int fontSizePause = 30;
            int centerXP = screenWidth/2 - MeasureText(pauseText, fontSizePause)/2;
            DrawTextWithBorder(pauseText, centerXP, screenHeight/3 - fontSizePause/2, fontSizePause, GREEN, BLACK);
            const char* resumeText = "Press P to Resume";
            int fontSizeResume = 20;
            DrawTextWithBorder(resumeText, screenWidth/2 - MeasureText(resumeText, fontSizeResume)/2, screenHeight/2 + 10, fontSizeResume, YELLOW, BLACK);
            const char* quitText = "Press ESC to Quit";
            DrawTextWithBorder(quitText, screenWidth/2 - MeasureText(quitText, fontSizeResume)/2, screenHeight/2 + 40, fontSizeResume, WHITE, BLACK);
            EndDrawing();
        }

        else if (state == GAMEOVER) {
            BeginDrawing();
            DrawTexture(gameOverBg, 0, 0, WHITE);
            int fontSizeFinal = 20;
            const char* highScoreText = TextFormat("Highest Score: %i (%s)",
                                                   (difficulty==0 ? highScoreEasy : (difficulty==1 ? highScoreMedium : highScoreHard)),
                                                   difficultyName);
            DrawTextWithBorder(TextFormat("Final Score: %i", score),
                               screenWidth/2 - MeasureText(TextFormat("Final Score: %i", score), fontSizeFinal)/2,
                               screenHeight/2 + 40, fontSizeFinal, WHITE, BLACK);
            DrawTextWithBorder(highScoreText,
                               screenWidth/2 - MeasureText(highScoreText, fontSizeFinal)/2,
                               screenHeight/2 + 70, fontSizeFinal, YELLOW, BLACK);
            const char* restartText = "Press ENTER to Restart";
            DrawTextWithBorder(restartText,
                               screenWidth/2 - MeasureText(restartText, fontSizeFinal)/2,
                               screenHeight/2 + 100, fontSizeFinal, GREEN, BLACK);
            EndDrawing();

            if (IsKeyPressed(KEY_ENTER)) {
                lives = 1;
                score = 0;
                frameCounter = 0;
                enemies.clear();
                playerX = screenWidth / 2 - carSize / 2;
                roadOffset = 0;
                state = START;
                PlaySound(menuSelect);
            }
        }
    }

    UnloadSound(crashSound);
    UnloadSound(menuSelect);
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();

    UnloadTexture(playerCar);
    for (int i = 0; i < 4; i++) UnloadTexture(enemyCars[i]);  // 🔹 unload new texture too
    UnloadTexture(roadTex);
    UnloadTexture(leftSideTex);
    UnloadTexture(rightSideTex);
    UnloadTexture(startBg);
    UnloadTexture(gameOverBg);
    UnloadTexture(pauseBg);
    CloseWindow();
    return 0;
}

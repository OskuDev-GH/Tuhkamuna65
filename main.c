#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

#define HIGHSCORE_PATH "high.bin"

Texture2D loadTexture(char *path, int width, int height);
int readHighscore();
void writeHighscore(int score);
int randint(int min, int max);
int game(void);

int windowWidth = 1440, windowHeight = 1080;

const int screenWidth = 960, screenHeight = 720;

int highscore = 0;

Texture2D panda, cockroach, scaryFace;
Sound deathSound, pointSound;
Music music;

int main(void) {
    InitWindow(windowWidth, windowHeight, "Tuhkamuna 65");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    InitAudioDevice();
    SetTargetFPS(0);

    panda = loadTexture("assets/panda man.png", screenWidth, screenHeight);
    cockroach = loadTexture("assets/cockroach.png", 150, 500);
    scaryFace = loadTexture("assets/scary face.png", 100, 130);

    deathSound = LoadSound("assets/death2.wav");
    pointSound = LoadSound("assets/point.wav");
    music = LoadMusicStream("assets/music.wav");

    int result;
    loop:
    result = game();
    if (result == -1) {
        goto loop;
    }
    else {
        UnloadSound(deathSound);
        UnloadSound(pointSound);
        UnloadMusicStream(music);

        CloseWindow();
        return result;
    }
}

Texture2D loadTexture(char *path, int width, int height) {
    Image image = LoadImage(path);
    ImageResize(&image, width, height);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    return texture;
}

int readHighscore() {
    FILE *file = fopen(HIGHSCORE_PATH, "rb");
    if (file == NULL) {
        printf("Could not open highscore file!\n");

        return 0;
    }
    else {
        int buffer[1];
        fread(buffer, sizeof(int), 1, file);
        fclose(file);

        return buffer[0];
    }
}

void writeHighscore(int score) {
    FILE *file = fopen(HIGHSCORE_PATH, "wb");
    if (file == NULL) {
        printf("Could not open highscore file!\n");
    }
    else {
        fwrite((int[]){ score }, sizeof(int), 5, file);

        fclose(file);
    }
}

int randint(int min, int max) {
    srand(time(NULL));
    return (rand() + min) % max;
}

int game(void) {
    highscore = readHighscore();

    Vector2 playerPosition = { screenWidth / 4.0, screenHeight / 2.0 };

    int speed = 100;
    double playerVelocityX = 0;
    double playerVelocityY = 0;

    int obstacleGap = 750; //gap between top and bottom obstacle things
    Vector2 obstacle1Pos = { screenWidth / 2.0 - cockroach.width / 2.0, screenHeight / 2.0 - cockroach.height / 2.0 };
    Vector2 obstacle2Pos = { screenWidth, screenHeight / 2.0 - cockroach.height / 2.0 };
    
    int paused = 0;
    int gameStarted = 0;
    int dead = 0;

    int deathSoundPlayed = 0;
    
    int points = 0;
    int obs1Point = 0;
    int obs2Point = 0;

    int newHighscore = 0;

    PlayMusicStream(music);

    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

    while (!WindowShouldClose()) {
        UpdateMusicStream(music);

        float dt = GetFrameTime();

        BeginTextureMode(target);
        DrawTexture(panda, screenWidth / 2 - panda.width / 2, screenHeight / 2 - panda.height / 2, WHITE);

        float obs1X = obstacle1Pos.x - (float)cockroach.width / 2;
        float obs2X = obstacle2Pos.x - (float)cockroach.width / 2;

        DrawTexture(cockroach, obs1X, obstacle1Pos.y + obstacleGap / 2.0, WHITE);
        DrawTexture(cockroach, obs1X, obstacle1Pos.y - obstacleGap / 2.0, WHITE);

        DrawTexture(cockroach, obs2X, obstacle2Pos.y + obstacleGap / 2.0, WHITE);
        DrawTexture(cockroach, obs2X, obstacle2Pos.y - obstacleGap / 2.0, WHITE);
        
        //no touchy
        Vector2 playerDrawPosition = { playerPosition.x - scaryFace.width / 2.0, playerPosition.y - scaryFace.height / 2.0 };
        Rectangle playerSource = { 0, 0, scaryFace.width, scaryFace.height };
        Rectangle playerDest = { playerPosition.x, playerPosition.y, scaryFace.width, scaryFace.height };
        Vector2 playerOrigin = { scaryFace.width / 2.0, scaryFace.height / 2.0 };
        DrawTexturePro(scaryFace, playerSource, playerDest, playerOrigin, -playerVelocityY / 20, WHITE);

        if (gameStarted) {
            if (!paused) {
                playerVelocityY += 10 * speed * dt;
                playerPosition.x += dt * playerVelocityX;
                playerPosition.y += dt * playerVelocityY;
                
                if (playerPosition.y < 0 || playerPosition.y > screenHeight) {
                    playerVelocityX = 500;
                    playerVelocityY = -500;
                    dead = 1;
                }
            }

            if (!dead) {
                if (IsKeyPressed(KEY_P)) {
                    paused = !paused;

                    if (paused) {
                        PauseMusicStream(music);
                    }
                    else {
                        PlayMusicStream(music);
                    }
                }

                int jump = !paused && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE));
            
                if (jump) {
                    playerVelocityY = -3 * speed;
                }

                if (paused) {
                    DrawText("game is pause", screenWidth / 10, screenHeight / 2 - 75, 100, BLUE);
                    DrawText("no (press P to unpause)", screenWidth / 10, screenHeight / 2 + 25, 50, DARKBLUE);
                }
                else {
                    obstacle1Pos.x -= 2 * speed * dt;
                    obstacle2Pos.x -= 2 * speed * dt;
                }

                if (obstacle1Pos.x < 0 - cockroach.width / 2.0) {
                    obstacle1Pos.x = screenWidth + cockroach.width / 2.0;
                    obstacle1Pos.y = randint(screenHeight / 2.0 - cockroach.height / 2.0 - 100, screenHeight / 2.0 - cockroach.height / 2.0 + 100);
                }

                if (obstacle2Pos.x < 0 - cockroach.width / 2.0) {
                    obstacle2Pos.x = screenWidth + cockroach.width / 2.0;
                    obstacle2Pos.y = randint(screenHeight / 2.0 - cockroach.height / 2.0 - 100, screenHeight / 2.0 - cockroach.height / 2.0 + 100);
                }

                if (playerPosition.x > obs1X - 25 && playerPosition.x < obs1X + 25) {
                    if (!obs1Point) {
                        obs1Point = 1;
                        ++points;
                        PlaySound(pointSound);
                    }
                }
                else {
                    obs1Point = 0;
                }

                if (playerPosition.x > obs2X - 25 && playerPosition.x < obs2X + 25) {
                    if (!obs2Point) {
                        obs2Point = 1;
                        ++points;
                        PlaySound(pointSound);
                    }
                }
                else {
                    obs2Point = 0;
                }

                Rectangle playerRect = (Rectangle){
                    playerPosition.x - scaryFace.width / 2.0, playerPosition.y - scaryFace.height / 2.0,
                    scaryFace.width, scaryFace.height
                };

                Rectangle obstacleRects[4] = {
                    (Rectangle){
                        obs1X, obstacle1Pos.y + obstacleGap / 2.0,
                        cockroach.width, cockroach.height
                    },
                    (Rectangle){
                        obs1X, obstacle1Pos.y - obstacleGap / 2.0,
                        cockroach.width, cockroach.height
                    },
                    (Rectangle){
                        obs2X, obstacle2Pos.y + obstacleGap / 2.0,
                        cockroach.width, cockroach.height
                    },
                    (Rectangle){
                        obs2X, obstacle2Pos.y - obstacleGap / 2.0,
                        cockroach.width, cockroach.height
                    },
                };

                for (int i = 0; i < 4; ++i) {
                    if (CheckCollisionRecs(playerRect, obstacleRects[i])) {
                        dead = 1;
                        playerVelocityX = 500;
                        playerVelocityY = -500;
                        break;
                    }
                }
            }
            else {
                if (!deathSoundPlayed) {
                    deathSoundPlayed = 1;
                    StopMusicStream(music);
                    PlaySound(deathSound);

                    if (highscore < points) {
                        writeHighscore(points);
                        newHighscore = 1;
                    }
                }

                if (newHighscore) {
                    DrawText("NEW HIGHSCORE!", screenWidth / 10, screenHeight / 2 - 175, 50, GREEN);
                }
                
                const char *pointText = TextFormat("points: %i", points);
                DrawText(pointText, screenWidth / 10, screenHeight / 2 - 125, 50, MAROON);
                DrawText("tuhkamuna is kil", screenWidth / 10, screenHeight / 2 - 75, 100, RED);
                DrawText("no (press R to restart)", screenWidth / 10, screenHeight / 2 + 25, 50, MAROON);

                if (IsKeyPressed(KEY_R)) {
                    return -1;
                }
            }
        }
        else {
            DrawText("press S to start", screenWidth / 10, screenHeight / 2 - 50, 90, GREEN);

            if (IsKeyPressed(KEY_S)) {
                gameStarted = 1;
            }
        }

        const char *pointText = TextFormat("Points: %i", points);
        DrawRectangle(0, 0, 14 * strlen(pointText), 30, BLACK);
        DrawText(pointText, 5, 5, 20, WHITE);

        const char *highscoreText = TextFormat("Highscore: %i", highscore);
        DrawRectangle(0, 30, 14 * strlen(highscoreText), 30, BLACK);
        DrawText(highscoreText, 5, 35, 20, WHITE);

        const char *fpsText = TextFormat("FPS: %i", GetFPS());
        DrawRectangle(screenWidth - 14 * strlen(fpsText), 0, 14 * strlen(fpsText), 30, BLACK);
        DrawText(fpsText, screenWidth - 14 * strlen(fpsText) + 5, 5, 20, WHITE);
        
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        windowWidth = GetScreenWidth();
        windowHeight = GetScreenHeight();

        Rectangle source = { 0, 0, (float)target.texture.width, (float)-target.texture.height };

        //magic
        float ratio = (float)screenWidth / screenHeight;
        Rectangle dest;
        if ((float)windowWidth / windowHeight >= ratio) {
            ratio = (float)screenWidth / screenHeight;
            dest = (Rectangle){ (windowWidth - ratio * windowHeight) / 2, 0, ratio * windowHeight, windowHeight };
        }
        else {
            ratio = (float)screenHeight / screenWidth;
            dest = (Rectangle){ 0, (windowHeight - ratio * windowWidth) / 2, windowWidth, ratio * windowWidth };
        }

        DrawTexturePro(target.texture, source, dest, (Vector2){ 0, 0 }, 0, WHITE);

        EndDrawing();
    }
    
    return 0;
}
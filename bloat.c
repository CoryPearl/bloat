// Made by Cory Pearl
// Start date: 03/17/26
// Version 3.0

// Hours worked: 
// 22 coding 
// 4 desigining 
// 5 trying to complie to web asemmbley

// TODO:
// - Make gamestate struct
// - Make active/inactive bool for bodys, enimies, cutbodys, and pickups struct

// IDEAS:
// - A third enemy type that bounces off walls instead of homing

// Remember:
// - 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <raylib.h>

#define MAX_BODYS 99
#define MAX_CUT_BODYS 99
#define MAX_PICKUPS 100
#define MAX_ENIMIES 100
#define HISTORY_SIZE 1000
#define BASE_SPEED 125
#define BASE_TURN_SPEED 5

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float x;
    float y;
    float theta;
    float speed;
    float turnSpeed;
    Color color;
    int size;
    bool shedReady;
    float speedBoostTimer;
    float freezeTimer;
    float scoreMultiplierTimer;
} Player;

typedef struct {
    float x;
    float y;
    float r;
    Color color;
} Body;

typedef struct {
    float x;
    float y;
    float r;
    Color color;
    float timer;
} Cutbody;

typedef struct {
    float x;
    float y;
    float r;
    bool growing;
    int kind;
} Pickup;

typedef struct {
    float x;
    float y;
    float speed;
    float r;
    float theta;
    int targetIndex;
    int type;
    Color color;
} Enimie;

float randomRange(float n1, float n2) {
    return n1 + ((float)rand() / (float)RAND_MAX) * (n2 - n1);
}

float distance(float x1, float y1, float x2, float y2) {
    return ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
}

float distanceWithRadius(float x1, float y1, float x2, float y2, int r1, int r2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float radiusSum = r1 + r2;

    return (dx * dx + dy * dy) - (radiusSum * radiusSum);
}

void updateClock(float *ticksAlive, float *timeAlive, float *ticksSinceShed, float *timeSinceShed, float *fps, bool *shedReady, Player *player, bool speedBoost) {
    *ticksAlive += 1;
    *timeAlive = *ticksAlive / *fps;

    if (!*shedReady) {
        *ticksSinceShed += 1;
        *timeSinceShed = *ticksSinceShed / *fps;
    }

    if (*timeSinceShed > 4 && !speedBoost) {  // only reset if no boost active
        player->speed = BASE_SPEED;
    }

    if (*timeSinceShed > 4 && !*shedReady && player->size >= 6) {
        *shedReady = true;
        player->shedReady = true;
    }
}

void updateBoostTimers(Player *player, bool *speedBoost, bool *freeze, bool *scoreMultiplier, Enimie *enimies, float dt) {
    // Speed boost
    if (*speedBoost) {
        player->speedBoostTimer -= dt;
        if (player->speedBoostTimer <= 0) {
            *speedBoost = false;
            player->speedBoostTimer = 0;
            player->speed = BASE_SPEED;
        }
    }

    // Freeze
    if (*freeze) {
        player->freezeTimer -= dt;
        for (int i = 0; i < MAX_ENIMIES; i++) {
            if (enimies[i].x != 0) enimies[i].speed = 0;
        }
        if (player->freezeTimer <= 0) {
            *freeze = false;
            player->freezeTimer = 0;
            // Change back enemy speeds
            for (int i = 0; i < MAX_ENIMIES; i++) {
                if (enimies[i].x != 0) {
                    enimies[i].speed = 100 - (enimies[i].r * 3);
                }
            }
        }
    }

    // Score multiplier
    if (*scoreMultiplier) {
        player->scoreMultiplierTimer -= dt;
        if (player->scoreMultiplierTimer <= 0) {
            *scoreMultiplier = false;
            player->scoreMultiplierTimer = 0;
        }
    }
}

bool checkCollision(float x1, float y1, float x2, float y2, int r1, int r2 ) {
    float dSquared = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
    float rSumSquared = (r1 + r2) * (r1 + r2);

    return dSquared <= rSumSquared;
}

void drawStartScreen(int screenWidth, int screenHeight, int margin, int gameWidth, int gameHeight, bool *gameStart, Color BACKGROUND, Color ACCENT, float *startMenuTitleOffset, bool *startMenuTitleGrowing, bool *suppressSpaceToPlay, Texture2D soundImg, float soundRotation, int *colorPickedIndex, Player *player) {
    /* Same Space press that left game-over must not immediately start a new run */
    if (*suppressSpaceToPlay) {
        *suppressSpaceToPlay = false;
    } else if (IsKeyPressed(KEY_SPACE)) {
        *gameStart = true;
    }

    ClearBackground(BACKGROUND);

    DrawRectangle(0, gameHeight + margin / 2, screenWidth, margin / 2, (Color){25,25,25,255});

    DrawText("Made By Cory Pearl", 10, 10, 20, LIGHTGRAY);

    DrawText("BLOAT", screenWidth / 2 - MeasureText("BLOAT", 150) / 2, screenHeight / 2 - margin * 1.5 - *startMenuTitleOffset, 150, LIGHTGRAY);
    DrawText("ARCADE SURVIVAL", screenWidth / 2 - MeasureText("ARCADE SURVIVAL", 20) / 2, screenHeight / 2 - margin * 2 , 20, ACCENT);

    DrawText("GROW YOUR SCORE  -  LOSE YOUR SPEED  -  SHED TO SURVIVE  -  ENEMIES TARGET YOUR BODY", screenWidth / 2 - MeasureText("GROW YOUR SCORE  -  LOSE YOUR SPEED  -  SHED TO SURVIVE  -  ENEMIES TARGET YOUR BODY", 12) / 2, screenHeight - 25, 12, DARKGRAY);

    DrawText("your score is ", screenWidth / 2 - MeasureText("your score is physical  -  it drags behind you", 15) / 2,screenHeight / 2 - margin * 1.5 + 130, 15, DARKGRAY);
    DrawText("physical",  screenWidth / 2 - MeasureText("physical", 15),screenHeight / 2 - margin * 1.5 + 130, 15, ACCENT);
    DrawText("  -  it drags behind you", screenWidth / 2,screenHeight / 2 - margin * 1.5 + 130, 15, DARKGRAY);

    DrawText("the more you earn  -  the harder it gets", screenWidth / 2 - MeasureText("the more you earn  -  the harder it gets", 15) / 2,screenHeight / 2 - margin * 1.5 + 160, 15, DARKGRAY);

    DrawRectangle(screenWidth / 2 - 150, screenHeight / 2 - margin * 1.5 + 210, 300, 40, ACCENT);
    DrawText("PRESS SPACE TO PLAY", screenWidth / 2 - MeasureText("PRESS SPACE TO PLAY", 15) / 2, screenHeight / 2 - margin * 1.5 + 223, 15, BACKGROUND );

    // a d <- ->
    Rectangle rect1 = {20, gameHeight - 30, 50, 25};
    DrawRectangleLinesEx(rect1, 1, DARKGRAY);
    DrawText("A   D", 28, gameHeight - 25, 15, LIGHTGRAY);
    DrawText("<-  ->", 83, gameHeight - 25, 15, LIGHTGRAY);
    DrawText("TURN", 60, gameHeight , 10, LIGHTGRAY);
    Rectangle rect2 = {75, gameHeight - 30, 50, 25};
    DrawRectangleLinesEx(rect2, 1, DARKGRAY);

    // Space to shed
    Rectangle rect3 = {150, gameHeight - 30, 75, 25};
    DrawRectangleLinesEx(rect3, 1, DARKGRAY);
    DrawText("SPACE", 163, gameHeight - 25, 15, LIGHTGRAY);
    DrawText("SHED", 175, gameHeight , 10, LIGHTGRAY);

    // R to restart
    Rectangle rect4 = {250, gameHeight - 30, 25, 25};
    DrawRectangleLinesEx(rect4, 1, DARKGRAY);

    // Border
    Rectangle rect = {1, margin / 2, gameWidth - 2, gameHeight};
    DrawRectangleLinesEx(rect, 1, DARKGRAY);
    DrawText("R", 258, gameHeight - 25, 15, LIGHTGRAY);
    DrawText("RESTART", 240, gameHeight , 10, LIGHTGRAY);

    Color colors[6] = {
        (Color){255, 28, 39,255},
        (Color){230,103,0,255},
        (Color){245,221,0,255},
        (Color){120,199,68,255},
        (Color){0,155,245,255},
        (Color){245,98,196,255}
    };

    player->color = colors[*colorPickedIndex];

    Rectangle rect5 = {screenWidth - 95, 8, 25, 25};
    DrawRectangleLinesEx(rect5, 2, DARKGRAY);
    DrawText("<-", screenWidth - 90, 11, 20, LIGHTGRAY);

    DrawRectangle(screenWidth - 65, 8, 25, 25, player->color);

    Rectangle rect6 = {screenWidth - 35, 8, 25, 25};
    DrawRectangleLinesEx(rect6, 2, DARKGRAY);
    DrawText("->", screenWidth - 30, 11, 20, LIGHTGRAY);

    // Bouncing text
    float speed = 25.0f; // pixels per second

    if (*startMenuTitleGrowing) {
        *startMenuTitleOffset += speed * GetFrameTime();
    } else {
        *startMenuTitleOffset -= speed * GetFrameTime();
    }

    if (*startMenuTitleOffset > 30) {
        *startMenuTitleGrowing = false;
    } else if (*startMenuTitleOffset < 0) {
        *startMenuTitleGrowing = true;
    }

    Rectangle soundSrc = { 0, 0, (float)soundImg.width, (float)soundImg.height };
    Rectangle soundDest = {
        screenWidth - soundImg.width - 10 + soundImg.width / 2.0f,
        screenHeight - soundImg.height - (margin / 2) - 10 + soundImg.height / 2.0f,
        soundImg.width,
        soundImg.height
    };
    Vector2 soundOrigin = { soundImg.width / 2.0f, soundImg.height / 2.0f };
    DrawTexturePro(soundImg, soundSrc, soundDest, soundOrigin, soundRotation, WHITE);

}

void drawGameScreen(int screenWidth, int screenHeight, int margin, int gameWidth, int gameHeight, int cellSize, int score, int wave, float timeAlive, float timeSinceShed, bool shedReady, Player player, Color BACKGROUND, Color GRID_LINES, Color ACCENT, Pickup *pickups, int pickupCount, Body *bodys, int *historyIndex, Vec2 *history, Cutbody *cutbodys, Enimie *enimies, bool speedBoost, bool scoreMultiplier, bool freeze) {    
    ClearBackground(BACKGROUND);

    // Vertical lines
    for (int x = 2; x <= gameWidth; x += cellSize) {
        DrawLine(x, margin / 2, x, margin / 2 + gameHeight, GRID_LINES);
    }

    // Horizontal lines
    for (int y = margin / 2; y <= margin / 2 + gameHeight; y += cellSize) {
        DrawLine(2, y, gameWidth - 2, y, GRID_LINES);
    }

    // Corner accents

    // Top left
    DrawRectangle(8, 8, 15, 1, ACCENT);
    DrawRectangle(8, 8, 1, 15, ACCENT);

    // Bottom left
    DrawRectangle(8, screenHeight - 8, 15, 1, ACCENT);
    DrawRectangle(8, screenHeight - 8 - 15, 1, 15, ACCENT);

    // Top right
    DrawRectangle(screenWidth - 8 - 15, 8, 15, 1, ACCENT);
    DrawRectangle(screenWidth - 8, 8, 1, 15, ACCENT);
    
    // Top right
    DrawRectangle(screenWidth - 8 - 15, screenHeight - 8, 15, 1, ACCENT);
    DrawRectangle(screenWidth - 8, screenHeight - 8 - 15, 1, 15, ACCENT);

    // Score
    DrawText(TextFormat("%i", score), 16, 12, 20, LIGHTGRAY);

    // Multiplier
    // int textWidthMultiplier = MeasureText(TextFormat("%i", score), 20);
    // int textHeightMultiplier = MeasureText(TextFormat("x%i", multiplier), 20);
    DrawRectangle(MeasureText(TextFormat("%i", score), 20) + 16 + 5, 12, MeasureText(TextFormat("x%i", player.size), 20), 20, ACCENT);
    DrawText(TextFormat("x%i", player.size), MeasureText(TextFormat("%i", score), 20) + 16 + 5 + 3, 12 + 3, 15, BLACK);

    // Wave
    // int textWidthWave = MeasureText(TextFormat("WAVE %i", wave), 20);
    DrawText(TextFormat("WAVE %i", wave), (screenWidth / 2) - (MeasureText(TextFormat("WAVE %i", wave), 20) / 2), 12 , 20, LIGHTGRAY);

    // Time alive
    // int textWidthTime = MeasureText(TextFormat("TIME %.2f", timeAlive), 20);
    DrawText(TextFormat("TIME %.2f", timeAlive), screenWidth - MeasureText(TextFormat("TIME %.2f", timeAlive), 20) - 16, 12, 20, LIGHTGRAY);

    // ESC to pause
    // int escWidth = MeasureText("| ESC | pause", 15);
    DrawText("[ ESC ]  pause", gameWidth - MeasureText("[ ESC ] pause", 15) - 13, margin / 2 + 10, 15, DARKGRAY);

    // Draw powerup timers
    // case 1:
    //     color = (Color){255, 119, 0, 255};
    //     break;
    // case 2:
    //     color = (Color){255, 48, 238, 255};
    //     break;
    // case 3:
    //     color = (Color){52, 235, 222, 255};
    //     break;
    // }

    int activePowerUps = 0;
    if (speedBoost) activePowerUps += 1;
    if (scoreMultiplier) activePowerUps += 1;
    if (freeze) activePowerUps += 1;

    int offsetMargin = 0;

    // activePowerUps = 3;
    // speedBoost = true;
    // scoreMultiplier = true;
    // freeze = true;

    if (activePowerUps > 0) {
        
        if (speedBoost) {
            DrawRectangle(gameWidth - MeasureText("[ ESC ] pause", 15) - 10, margin / 2 + 30 + (offsetMargin * 25), MeasureText("[ ESC ] pause", 15) + 5, 6, DARKGRAY);
            DrawRectangle(gameWidth - MeasureText("[ ESC ] pause", 15) - 10, margin / 2 + 30 + (offsetMargin * 25), player.speedBoostTimer / 4.0f * (MeasureText("[ ESC ] pause", 15) + 5), 6, (Color){255, 119, 0, 255});
            DrawText("SPEED", gameWidth - MeasureText("[ ESC ] pause", 10) - MeasureText("SPEED",10) * 3 + 5, margin / 2 + 28 + (offsetMargin * 25), 15, (Color){255, 119, 0, 255});
            offsetMargin += 1;
        }

        if (scoreMultiplier) {
            DrawRectangle(gameWidth - MeasureText("[ ESC ] pause", 15) - 10, margin / 2 + 30 + (offsetMargin * 25), MeasureText("[ ESC ] pause", 15) + 5, 6, DARKGRAY);
            DrawRectangle(gameWidth - MeasureText("[ ESC ] pause", 15) - 10, margin / 2 + 30 + (offsetMargin * 25), player.scoreMultiplierTimer/ 8.0f * (MeasureText("[ ESC ] pause", 15) + 5), 6, (Color){255, 48, 238, 255});
            DrawText("10 X SCORE", gameWidth - MeasureText("[ ESC ] pause", 10) - MeasureText("10X SCORE",10) * 2 - 20, margin / 2 + 28 + (offsetMargin * 25), 15, (Color){255, 48, 238, 255});

            offsetMargin += 1;
        }

        if (freeze) {
            DrawRectangle(gameWidth - MeasureText("[ ESC ] pause", 15) - 10, margin / 2 + 30 + (offsetMargin * 25), MeasureText("[ ESC ] pause", 15) + 5, 6, DARKGRAY);
            DrawRectangle(gameWidth - MeasureText("[ ESC ] pause", 15) - 10, margin / 2 + 30 + (offsetMargin * 25), player.freezeTimer / 2.0f * (MeasureText("[ ESC ] pause", 15) + 5), 6, (Color){52, 235, 222, 255});
            DrawText("FREEZE", gameWidth - MeasureText("[ ESC ] pause", 10) - MeasureText("FREEZE",10) * 2 - 23, margin / 2 + 28 + (offsetMargin * 25), 15, (Color){52, 235, 222, 255});
        }

    }

    // Border box
    Rectangle rect = {1, margin / 2, gameWidth - 2, gameHeight};
    DrawRectangleLinesEx(rect, 1, DARKGRAY);

    // Shed ready
    const char *shedText;
    if (player.size < 6) {
        shedText = TextFormat("SHED (NEED %i MORE)", 6 - player.size);
    } else {
        if (shedReady) {
            shedText = "SHED READY";
        } else {
            shedText = "SHED";
        }
    }

    int textWidthShed = MeasureText(shedText, 15);

    if ( shedReady && player.size >= 6) {
        DrawText(shedText, (screenWidth / 2) - (textWidthShed) - 5, screenHeight - margin / 4 - 8, 15, ACCENT);
    } else {
        DrawText(shedText, (screenWidth / 2) - (textWidthShed) - 5, screenHeight - margin / 4 - 8, 15, DARKGRAY);
    }

    DrawRectangle(screenWidth / 2 + 5, screenHeight - margin / 4 - 4, 150, 6, DARKGRAY);
    if ( timeSinceShed <= 4 ) {
        DrawRectangle(screenWidth / 2 + 5, screenHeight - margin / 4 - 4, 37.5 * timeSinceShed, 6, ACCENT);
    } else {
        DrawRectangle(screenWidth / 2 + 5, screenHeight - margin / 4 - 4, 37.5 * 4, 6, ACCENT);
    }

    int pressSpaceWidth = MeasureText("space", 20);
    Rectangle rectForSpace = {screenWidth / 2 + 5 + 155, screenHeight - margin / 4 - 10, pressSpaceWidth, 20};
    DrawRectangleLinesEx(rectForSpace, 1, DARKGRAY);
    DrawText("space", screenWidth / 2 + 5 + 155 + 8,  screenHeight - margin / 4 - 8 ,15, LIGHTGRAY);

    // Draw player
    DrawCircle(player.x, player.y, 15, LIGHTGRAY);
    // Direction x = speed * cos(theta)
    // Direction y = speed * sin(theta)
    // DrawCircle(player.x + (cosf(player.theta) * 7.5), player.y + (sinf(player.theta) * 7.5), 5, BLACK);

    Vector2 headPosition = { player.x, player.y };
    DrawCircleV(headPosition, 15, LIGHTGRAY);

    Vector2 pipPosition = {
        player.x + cosf(player.theta) * 7.5f,
        player.y + sinf(player.theta) * 7.5f
    };
    DrawCircleV(pipPosition, 5, BLACK);

    // Draw pickups 
    for (int i = 0; i < MAX_PICKUPS; i++) {
        if (pickups[i].x != 0 && pickups[i].y != 0) {
            Color color = ACCENT;

            if (pickups[i].kind != 0) {
                switch (pickups[i].kind) {
                    case 1:
                        color = (Color){255, 119, 0, 255};
                        break;
                    case 2:
                        color = (Color){255, 48, 238, 255};
                        break;
                    case 3:
                        color = (Color){52, 235, 222, 255};
                        break;
                }
            }

            DrawCircle(pickups[i].x, pickups[i].y, 5, color);
            DrawCircleLines(pickups[i].x, pickups[i].y, pickups[i].r, color);
        }
    }

    // Draw bodys
    for (int i = 0; i < MAX_BODYS; i++) {
        if (bodys[i].x != 0 && bodys[i].y != 0) {
            // DrawCircle(bodys[i].x, bodys[i].y, bodys[i].r, bodys[i].color);
            DrawCircleV((Vector2){ bodys[i].x, bodys[i].y }, bodys[i].r, bodys[i].color); // Fix jitter
        }
    }

    // Draw cut bodys
    for (int i = 0; i < MAX_CUT_BODYS; i++) {
        if (cutbodys[i].x != 0 && cutbodys[i].y != 0) {
            // DrawCircle(cutbodys[i].x, cutbodys[i].y, cutbodys[i].r, cutbodys[i].color);
            DrawCircleV((Vector2){ cutbodys[i].x, cutbodys[i].y }, cutbodys[i].r, cutbodys[i].color); // Fix jitter
        }
    }

    // Draw enimies
    for (int i = 0; i < MAX_ENIMIES; i++) {
        if (enimies[i].x != 0 && enimies[i].y != 0) {
            DrawCircle(enimies[i].x, enimies[i].y, enimies[i].r, enimies[i].color);
            DrawCircle(enimies[i].x, enimies[i].y, enimies[i].r - 1, BLACK);
        }
    }

}

void drawPausedScreen(int screenWidth, int screenHeight) {
    Color dimmer = (Color){0, 0, 0, 100};
    DrawRectangle(0, 0, screenWidth, screenHeight, dimmer);

    int pausedFontSize = 30;
    int hintFontSize = 20;

    int cy = screenHeight / 2 - 40;
    DrawText("PAUSED", screenWidth / 2 - MeasureText("PAUSED", pausedFontSize) / 2, cy, pausedFontSize, LIGHTGRAY);
    cy += pausedFontSize + 24;
    DrawText("PRESS ESC TO RESUME", screenWidth / 2 - MeasureText("PRESS ESC TO RESUME", hintFontSize) / 2, cy, hintFontSize, LIGHTGRAY);
    cy += hintFontSize + 14;
    DrawText("PRESS SPACE FOR MAIN MENU", screenWidth / 2 - MeasureText("PRESS SPACE FOR MAIN MENU", hintFontSize) / 2, cy, hintFontSize, LIGHTGRAY);
}

void drawRestartScreen(int screenWidth, int screenHeight, Color ACCET, int score, int wave, float time, int length, int maxLength) {
    Color dimmer = (Color){0, 0, 0, 100};
    DrawRectangle(0, 0, screenWidth, screenHeight, dimmer);

    DrawText("YOU LOSE", screenWidth/2 - MeasureText("YOU LOSE", 70) / 2, screenHeight / 2 - 150, 70, ACCET);
    DrawText(TextFormat("FINAL LENGTH %i", length), screenWidth / 2 - MeasureText(TextFormat("FINAL LENGTH %i", length), 30) / 2, screenHeight / 2 - 80, 30, LIGHTGRAY);
    DrawText(TextFormat("MAX LENGTH %i", maxLength), screenWidth / 2 - MeasureText(TextFormat("MAX LENGTH %i", maxLength), 30) / 2, screenHeight / 2 - 40, 30, LIGHTGRAY);
    DrawText(TextFormat("FINAL TIME %.2f", time), screenWidth / 2 - MeasureText(TextFormat("FINAL TIME %.2f", time), 30) / 2, screenHeight / 2, 30, LIGHTGRAY);
    DrawText(TextFormat("FINAL SCORE %i", score), screenWidth / 2 - MeasureText(TextFormat("FINAL SCORE %i", score), 30) / 2, screenHeight / 2 + 40, 30, LIGHTGRAY);
    DrawText(TextFormat("FINAL WAVE %i", wave), screenWidth / 2 - MeasureText(TextFormat("FINAL WAVE %i", wave), 30) / 2, screenHeight / 2 + 80, 30, LIGHTGRAY);

    DrawText("PRESS R TO RESTART", screenWidth / 2 - MeasureText("PRESS R TO RESTART", 20) / 2, screenHeight / 2 + 160, 20, LIGHTGRAY);
    DrawText("OR", screenWidth / 2 - MeasureText("OR", 20) / 2, screenHeight / 2 + 190, 20, LIGHTGRAY);
    DrawText("SPACE TO RETURN TO MAIN MENU", screenWidth / 2 - MeasureText("SPACE TO RETURN TO MAIN MENU", 20) / 2, screenHeight / 2 + 220, 20, LIGHTGRAY);

}

void drawWinScreen(int screenWidth, int screenHeight, Color ACCET, int score, int wave, float time, int length, int maxLength) {
    Color dimmer = (Color){0, 0, 0, 100};
    DrawRectangle(0, 0, screenWidth, screenHeight, dimmer);

    DrawText("YOU WIN", screenWidth/2 - MeasureText("YOU WIN", 70) / 2, screenHeight / 2 - 150, 70, ACCET);
    DrawText(TextFormat("FINAL LENGTH %i", length), screenWidth / 2 - MeasureText(TextFormat("FINAL LENGTH %i", length), 30) / 2, screenHeight / 2 - 80, 30, LIGHTGRAY);
    DrawText(TextFormat("MAX LENGTH %i", maxLength), screenWidth / 2 - MeasureText(TextFormat("MAX LENGTH %i", maxLength), 30) / 2, screenHeight / 2 - 40, 30, LIGHTGRAY);
    DrawText(TextFormat("FINAL TIME %.2f", time), screenWidth / 2 - MeasureText(TextFormat("FINAL TIME %.2f", time), 30) / 2, screenHeight / 2, 30, LIGHTGRAY);
    DrawText(TextFormat("FINAL SCORE %i", score), screenWidth / 2 - MeasureText(TextFormat("FINAL SCORE %i", score), 30) / 2, screenHeight / 2 + 40, 30, LIGHTGRAY);
    DrawText(TextFormat("FINAL WAVE %i", wave), screenWidth / 2 - MeasureText(TextFormat("FINAL WAVE %i", wave), 30) / 2, screenHeight / 2 + 80, 30, LIGHTGRAY);

    DrawText("PRESS R TO RESTART", screenWidth / 2 - MeasureText("PRESS R TO RESTART", 20) / 2, screenHeight / 2 + 160, 20, LIGHTGRAY);
    DrawText("OR", screenWidth / 2 - MeasureText("OR", 20) / 2, screenHeight / 2 + 190, 20, LIGHTGRAY);
    DrawText("SPACE TO RETURN TO MAIN MENU", screenWidth / 2 - MeasureText("SPACE TO RETURN TO MAIN MENU", 20) / 2, screenHeight / 2 + 220, 20, LIGHTGRAY);

}

void shed(Player *player, float *ticksSinceShed, float *timeSinceShed, bool *shedReady, Body *bodys, Cutbody *cutbodys, int *cutbodyIndex, Sound shedWav) {
    if (*shedReady && player->shedReady && player->size > 1) {
        int totalToKeep = (player->size + 1) / 2; // keep half rounded up
        int totalToRemove = player->size - totalToKeep;

        // Move removed bodies to cutbodys
        for (int i = 0; i < totalToRemove; i++) {
            int removeIndex = totalToKeep - 1 + i; // index of body to remove
            cutbodys[*cutbodyIndex].x = bodys[removeIndex].x;
            cutbodys[*cutbodyIndex].y = bodys[removeIndex].y;
            cutbodys[*cutbodyIndex].r = bodys[removeIndex].r;
            cutbodys[*cutbodyIndex].color = bodys[removeIndex].color;
            cutbodys[*cutbodyIndex].timer = 4.0f;
            (*cutbodyIndex)++;

            // Clear the removed body
            bodys[removeIndex].x = 0;
            bodys[removeIndex].y = 0;
            bodys[removeIndex].r = 0;
            bodys[removeIndex].color = (Color){0,0,0,0};
        }

        // Compact remaining bodies to the front
        for (int i = 0; i < totalToKeep - 1; i++) {
            bodys[i] = bodys[i + totalToRemove];
        }

        player->size = totalToKeep;

        player->speed = 175;

        // Reset shed cooldown
        *ticksSinceShed = 0;
        *timeSinceShed = 0;
        *shedReady = false;
        player->shedReady = false;

        PlaySound(shedWav);
    }
}

void updateEntitiesAndValues(Player *player, float *ticksSinceShed, float *timeSinceShed, bool *shedReady, int gameWidth, int gameHeight, int margin, Pickup *pickups, int *pickupCount, int *wave, int *score, Body *bodys, int *historyIndex, Vec2 *history, Cutbody *cutbodys, int *cutbodyIndex, bool *paused, bool *lost, Enimie *enimies, Sound pickupWav, Sound shedWav, bool *won, bool *speedBoost, bool *scoreMultiplier, bool *freeze) { 
    float dt = GetFrameTime();

    updateBoostTimers(player, speedBoost, freeze, scoreMultiplier, enimies, dt);

    float currentTurnSpeed = BASE_TURN_SPEED / (1.0f + (player->size - 1) * 0.12f);

    // Check if any keys are down
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player->theta += currentTurnSpeed * dt;
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) player->theta -= currentTurnSpeed * dt;
    // if (IsKeyDown(KEY_ESCAPE)) *paused = true;
    if (IsKeyDown(KEY_SPACE)) shed(player, ticksSinceShed, timeSinceShed, shedReady, bodys, cutbodys, cutbodyIndex, shedWav);

    // Caclulate normalized speed based on direction of player
    float vx = cosf(player->theta);
    float vy = sinf(player->theta);

    // Apply calculated speed
    player->x += vx * player->speed * dt;
    player->y += vy * player->speed * dt;

    // Player bounds check
    if ( player->x < 2 + 15 || player->x > gameWidth - 15 ) {
        player->x -= vx * player->speed * dt;
    }

    if ( player->y < margin / 2 + 2 + 15|| player->y > gameHeight + margin / 2 - 15 ) {
        player->y -= vy * player->speed * dt;
    }

    history[*historyIndex] = (Vec2){ player->x, player->y };
    *historyIndex = (*historyIndex + 1) % HISTORY_SIZE;

    // Animate pickups and check collisions
    for (int i = 0; i < 99; i++) {
        if (pickups[i].x != 0 && pickups[i].y != 0) {

            if (pickups[i].growing) {
                pickups[i].r += 0.1;
                if ( pickups[i].r > 10) {
                    pickups[i].growing = false;
                }

            }

            if (!pickups[i].growing) {
                pickups[i].r -= 0.08;
                if ( pickups[i].r < 5) {
                    pickups[i].growing = true;
                }

            }

            if (checkCollision(player->x, player->y, pickups[i].x, pickups[i].y, 15, 5)) {
                pickups[i].x = 0;
                pickups[i].y = 0;
                pickups[i].r = 5;
                if (pickups[i].kind == 0) {
                    *pickupCount -= 1;
                    // *score += player->size * 10;s
                    *score += player->size * 10 * (*scoreMultiplier ? 10 : 1);
                    player->size += 1;
                } else {
                // speed boost : 3 seconds
                // freez : 2 seconds
                // score multiplier : 8 seconds
                // Add visual display with same color that pcikup was
                    switch (pickups[i].kind) {
                        case 1:
                            *speedBoost = true;
                            player->speed = 200;
                            player->speedBoostTimer = 4.0f;
                            break;
                        case 2:
                            *scoreMultiplier = true;
                            player->scoreMultiplierTimer = 8.0f;
                            break;
                        case 3:
                            *freeze = true;
                            player->freezeTimer = 2.0f;
                            break;
                    }
                }
                PlaySound(pickupWav);
            }
                // player->turnSpeed = BASE_TURN_SPEED - (player->size / 3);
        }
    }

    // Check head collisions with bodys and with cut bodys

    if ( *pickupCount == 0) {
        *wave += 1;
        if (*wave == 100) {
            *paused = true;
            *won = true;
        }

        player->speed = BASE_SPEED + *wave;

        // Random but also ramping up enimie count
        int base1 = 2 + (*wave / 2);
        int var1 = randomRange(0, (int)sqrt(*wave));

        int pcikupCountRand = base1 + var1;

        *pickupCount = pcikupCountRand;

        for (int i = 0; i < pcikupCountRand; i++) {
            pickups[i].x = randomRange(17, gameWidth);
            pickups[i].y = randomRange(17, gameHeight - margin) + margin / 2;
            pickups[i].r = 5;
            pickups[i].growing = true;

            int kind = 0;

            float randKind = randomRange(1,100);
            
            // randKind > 1.0f && randKind < 70.0f) stay normal
            // 1 = speed boost, 2 = score mmultiplier, 3 == freeze
            if (*wave > 3) {
                if (randKind > 87.5f && randKind < 92.5f){
                    kind = 1;
                    *pickupCount -= 1;
                } else if (randKind > 92.5f && randKind < 97.5f) {
                    kind = 2;
                    *pickupCount -= 1;
                } else if (randKind > 97.5f && randKind < 100.0f) {
                    kind = 3;
                    *pickupCount -= 1;
                }
            }

            pickups[i].kind = kind;

            while (checkCollision(pickups[i].x, pickups[i].y, player->x, player->y, 5, 15)) {
                pickups[i].x = randomRange(17, gameWidth);
                pickups[i].y = randomRange(17, gameHeight - margin) + margin / 2;
            }
        }

        for (int i = 0; i < MAX_ENIMIES; i++) {
            enimies[i].x = 0;
            enimies[i].y= 0;
        }

        // Random but also ramping up enimie count
        int base = 2 + (*wave / 2);
        int var = randomRange(0, (int)sqrt(*wave));

        int enemyCount = base + var;

        // Spawning random enimies
        for (int i = 0; i < enemyCount; i++) {

            enimies[i].x = randomRange(17, gameWidth);
            enimies[i].y = randomRange(17, gameHeight - margin) + margin / 2;
            // enimies[i].r = 10;
            enimies[i].r = randomRange(4,14);
            enimies[i].speed = 100 - (enimies[i].r * 3);
            enimies[i].type = 0;
            enimies[i].color = (Color){255, 59, 59, 255};

            while (checkCollision(enimies[i].x, enimies[i].y, player->x, player->y, enimies[i].r, 15) && distanceWithRadius(enimies[i].x, enimies[i].y, player->x, player->y, enimies[i].r, 15) > 200) {
                enimies[i].x = randomRange(17, gameWidth);
                enimies[i].y = randomRange(17, gameHeight - margin) + margin / 2;
            }

            float randEnimie = randomRange(1,100);

            if (randEnimie > 80.0f && randEnimie < 100.0f) {
                enimies[i].type = 1;
                enimies[i].color = (Color){3, 0, 158, 255};
                enimies[i].speed *= 1.75;
                enimies[i].theta = randomRange(1,360);
            }
        }
    }

    // Update positions of all bodys

    float baseSpacing = 17.5f;

    float currentSpacing = baseSpacing;

    float shrinkage = 1.01f;

    float offset = 0.0f;

    float baseAlpha = 255.0f;
    float currentAlpha = baseAlpha;

    for (int i = 0; i < player->size - 1; i++) {

        offset += currentSpacing;

        int index = *historyIndex - (int)offset;
        if (index < 0) index += HISTORY_SIZE;

        bodys[i].x = history[index].x;
        bodys[i].y = history[index].y;

        if (i == 0) {
            bodys[i].r = 15 / shrinkage;
        } else {
            bodys[i].r = bodys[i - 1].r / shrinkage;
        }

        currentAlpha /= shrinkage;

       bodys[i].color = (Color){ player->color.r, player->color.g, player->color.b, (unsigned char)currentAlpha };

        currentSpacing /= shrinkage;

        bool inBounds =
            player->x > 15 &&
            player->x < gameWidth - 15 &&
            player->y > (margin / 2) + 15 &&
            player->y < gameHeight + (margin / 2) - 15;

        if (inBounds && i > 2) {
            if (checkCollision(player->x, player->y, bodys[i].x, bodys[i].y, 15, bodys[i].r)) {
                *lost = true;
                *paused = true;
            }
        }
    }

    // Update cut bodys timers
    for (int i = 0; i < *cutbodyIndex; i++) {
        cutbodys[i].timer -= (1.0f / 60.0f);

        bool inBounds =
            player->x > 15 &&
            player->x < gameWidth - 15 &&
            player->y > (margin / 2) + 15 &&
            player->y < gameHeight + (margin / 2) - 15;

        if (cutbodys[i].x != 0 && cutbodys[i].y != 0 && inBounds) {
            if (checkCollision(player->x, player->y, cutbodys[i].x, cutbodys[i].y, 15, cutbodys[i].r)) {
                *lost = true;
                *paused = true;
            }
        }
        // Fade alpha only
        if (cutbodys[i].color.a > 0) {
            cutbodys[i].color.a -= 0.16f;
            if (cutbodys[i].color.a < 0) cutbodys[i].color.a = 0; 
        }

        if (cutbodys[i].timer < 0.16f) {
            cutbodys[i].x = 0;
            cutbodys[i].y = 0;
        }
    }

    // Enemies chase nearest point: head and/or tail segments (bodys updated above)
    int targetCounts[MAX_BODYS] = {0};

    for (int i = 0; i < MAX_ENIMIES; i++) {
        if (enimies[i].x == 0 || enimies[i].y == 0) continue;

        float tx = player->x;
        float ty = player->y;
        float minDist = distance(enimies[i].x, enimies[i].y, tx, ty);
        int bestSeg = -1; // -1 = targeting head //

        int maxPerSegment = 1;

        // for (int j = 0; j < player->size - 1; j++) {
        //     float d = distance(enimies[i].x, enimies[i].y, bodys[j].x, bodys[j].y);
        //     if (d < minDist) {
        //         minDist = d;
        //         tx = bodys[j].x;
        //         ty = bodys[j].y;
        //         bestSeg = j;
        //     }
        // }

        for (int j = 0; j < player->size - 1; j++) {
            // skip if this segment allready has a enimie
            if (targetCounts[j] >= maxPerSegment) continue;

            float d = distance(enimies[i].x, enimies[i].y, bodys[j].x, bodys[j].y);

            if (d < minDist) {
                minDist = d;
                bestSeg = j;
            }
        }

        if (bestSeg != -1) {
            targetCounts[bestSeg]++;
            tx = bodys[bestSeg].x;
            ty = bodys[bestSeg].y;
        } else {
            // fallback: target head
            tx = player->x;
            ty = player->y;
        }

        if (enimies[i].type == 0) {
            enimies[i].targetIndex = bestSeg;

            float dx = tx - enimies[i].x;
            float dy = ty - enimies[i].y;
            enimies[i].theta = atan2f(dy, dx);

            // Might not need this?
            // float dHead = distance(enimies[i].x, enimies[i].y, player->x, player->y);

            // if (dHead < minDist) {
            //     dx = tx - enimies[i].x;
            //     dy = ty - enimies[i].y;
            //     enimies[i].theta = atan2f(dy, dx);
            // }

            // Math to clculate direction and speed for enimies to travel in
            float evx = cosf(enimies[i].theta);
            float evy = sinf(enimies[i].theta);
            enimies[i].x += evx * enimies[i].speed * dt;
            enimies[i].y += evy * enimies[i].speed * dt;
        } else if (enimies[i].type == 1) {
            float evx = cosf(enimies[i].theta);
            float evy = sinf(enimies[i].theta);
            enimies[i].x += evx * enimies[i].speed * dt * 1.75;
            enimies[i].y += evy * enimies[i].speed * dt * 1.75;

            // Left wall
            if (enimies[i].x < 0 + enimies[i].r) {
                enimies[i].x = 0 + enimies[i].r;
                enimies[i].theta = randomRange(-M_PI / 2, M_PI / 2);
            }
            // Right wall
            if (enimies[i].x > gameWidth - enimies[i].r) {
                enimies[i].x = gameWidth - enimies[i].r;
                enimies[i].theta = randomRange(M_PI / 2, 3 * M_PI / 2);
            }
            // Top wall
            if (enimies[i].y < margin / 2 + enimies[i].r) {
                enimies[i].y = margin / 2 + enimies[i].r;
                enimies[i].theta = randomRange(0, M_PI);
            }
            // Bottom wall
            if (enimies[i].y > gameHeight + margin / 2 - enimies[i].r) {
                enimies[i].y = gameHeight + margin / 2 - enimies[i].r;
                enimies[i].theta = randomRange(-M_PI, 0);
            }
        }

        if (checkCollision(player->x, player->y, enimies[i].x, enimies[i].y, 15, enimies[i].r)) {
            *lost = true;
            *paused = true;
        }
    }
}

void restart(int screenWidth, int screenHeight, Player *player, Pickup *pickups, Body *bodys, Cutbody *cutbodys, Vec2 *history, int *historyIndex, int *score, int *wave, int *pickupCount, float *ticksAlive, float *timeAlive, float *ticksSinceShed, float *timeSinceShed, bool *gameStart, bool *paused, bool *escPressedLastFrame, float *startMenuTitleOffset, bool *startMenuTitleGrowing, bool *shedReady, bool *lost, int *maxLength, int *cutbodyIndex, Enimie *enimies) {

    player->x = (float)screenWidth / 2;
    player->y = (float)screenHeight /2;
    player->theta = 0.0f;
    player->turnSpeed = BASE_TURN_SPEED; // Turn speed in degrees per second
    player->speed = BASE_SPEED; // Speed in pixels per second
    player->size = 1;
    player->shedReady = false;
    player->speedBoostTimer = 0.0f;
    player->freezeTimer = 0.0f;
    player->scoreMultiplierTimer = 0.0f;

    for (int i = 0; i < MAX_PICKUPS; i++) {
        pickups[i].x = 0;
        pickups[i].y = 0;
        pickups[i].r = 5;
        pickups[i].growing = true;
    }

    for (int i = 0; i < MAX_BODYS; i++) {
        bodys[i].x = 0;
        bodys[i].y = 0;
    }

    for (int i = 0; i < MAX_CUT_BODYS; i++) {
        cutbodys[i].x = 0;
        cutbodys[i].y = 0;
    }
    *cutbodyIndex = 0;

    *historyIndex = 0;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i] = (Vec2){ player->x, player->y };
    }

    for (int i = 0; i < MAX_ENIMIES; i++) {
        enimies[i].x = 0;
        enimies[i].y = 0;
        enimies[i].r = 10;
        enimies[i].color = (Color){255, 59, 59, 255};
        enimies[i].type = 0;
    }

    *score = 0;
    *wave = 0;
    *pickupCount = 0;

    *ticksAlive = 0.0f;
    *timeAlive = 0.0f;

    *ticksSinceShed = 0.0f;
    *timeSinceShed = 0.0f; // Cooldown 4 seconds
    *shedReady = player->shedReady;

    *gameStart = true;
    *paused = false;
    *escPressedLastFrame = false;
    *lost = false;

    *startMenuTitleOffset = 0;
    *startMenuTitleGrowing = true;

    *maxLength = 0;

}

int main() {
    // Defiging constants for dimensions of screen
    const int screenWidth = 960;
    const int screenHeight = 720;
    const int margin = 80;
    const int gameWidth = screenWidth;
    const int gameHeight = screenHeight - margin;
    const int cellSize = 40;   // size of each square

    Color const BACKGROUND = {10,10,10,255};
    Color const ACCENT = {211,254,81,255};
    Color const GRID_LINES = {20,20,20,255};

    int colorPickedIndex = 3;

    Player player;
    player.x = (float)screenWidth / 2;
    player.y = (float)screenHeight /2;
    player.theta = 0.0f;
    player.turnSpeed = BASE_TURN_SPEED; // Turn speed in degrees per second
    player.speed = BASE_SPEED; // Speed in pixels per second
    player.size = 1;
    player.shedReady = false;
    player.speedBoostTimer = 0.0f;
    player.freezeTimer = 0.0f;
    player.scoreMultiplierTimer = 0.0f;
    player.color = (Color){120,199,68,255};

    Pickup pickups[MAX_PICKUPS]; // Max pickups 100, (wave 100 you beat the game)

    for (int i = 0; i < MAX_PICKUPS; i++) {
        pickups[i].x = 0;
        pickups[i].y = 0;
        pickups[i].r = 5;
        pickups[i].growing = true;
        pickups[i].kind = 0;
    }

    Body bodys[MAX_BODYS]; // Max bodys 100, (body 100 you beat the game)

    for (int i = 0; i < MAX_BODYS; i++) {
        bodys[i].x = 0;
        bodys[i].y = 0;
    }

    Cutbody cutbodys[MAX_CUT_BODYS]; // Max cut bodys 100, (body 100 you beat the game)
    int cutbodyIndex = 0;

    for (int i = 0; i < MAX_CUT_BODYS; i++) {
        cutbodys[i].x = 0;
        cutbodys[i].y = 0;
    }

    Vec2 history[HISTORY_SIZE]; // To store past positions
    int historyIndex = 0;

    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i] = (Vec2){ player.x, player.y };
    }

    Enimie enimies[MAX_ENIMIES];

    for (int i = 0; i < MAX_ENIMIES; i++) {
        enimies[i].x = 0;
        enimies[i].y = 0;
        enimies[i].r = 10;
        enimies[i].color = (Color){255, 59, 59, 255};
        enimies[i].type = 0;
    }

    enimies[0].x = 100;
    enimies[0].y = 100;

    int score = 0;
    int wave = 0;
    int pickupCount = 0;

    float ticksAlive = 0.0f;
    float timeAlive = 0.0f;

    float ticksSinceShed = 0.0f;
    float timeSinceShed = 0.0f; // Cooldown 4 seconds
    bool shedReady = player.shedReady;

    float fps = 60.0f;

    bool gameStart = false;
    bool paused = false;
    bool escPressedLastFrame = false;
    bool lost = false;
    bool won = false;
    bool speedBoost = false;
    bool freeze = false;
    bool scoreMultiplier = false;

    float startMenuTitleOffset = 0;
    bool startMenuTitleGrowing = true;

    int maxLength = 1;
    bool suppressSpaceToPlay = false;

    // Sound icon idle spin animation
    float soundRotation = 0.0f;
    float soundSpinTimer = 0.0f;
    float soundWaitTimer = randomRange(1.0f, 3.0f);
    bool soundSpinning = false;
    float soundSpinDuration = 0.0f;
    float soundSpinSpeed = 0.0f;

    InitWindow(screenWidth, screenHeight, "Bloat");

    InitAudioDevice();

    Music menuMusic = LoadMusicStream("menu.mp3");
    Music gameMusic = LoadMusicStream("game.mp3");

    PlayMusicStream(menuMusic);
    PlayMusicStream(gameMusic);

    Sound pickupWav = LoadSound("pickup.wav");
    Sound shedWav = LoadSound("shed.wav");

    Image soundImg = LoadImage("sound.png");
    ImageResize(&soundImg, 75, 75); 
    Texture2D soundImgTecture = LoadTextureFromImage(soundImg);

    SetTargetFPS(fps);

    SetExitKey(0);

    srand(time(NULL));

    while (!WindowShouldClose()) { // Game loop

        if (IsKeyPressed(KEY_SPACE) && lost) {
            restart(screenWidth, screenHeight, &player, pickups, bodys, cutbodys,
                history, &historyIndex, &score, &wave, &pickupCount,
                &ticksAlive, &timeAlive, &ticksSinceShed, &timeSinceShed,
                &gameStart, &paused, &escPressedLastFrame,
                &startMenuTitleOffset, &startMenuTitleGrowing,
                &shedReady, &lost, &maxLength, &cutbodyIndex, enimies);

            gameStart = false; /* main menu */
            suppressSpaceToPlay = true; /* don't treat same Space as "play" */
        } else if (IsKeyPressed(KEY_SPACE) && paused && !lost) {
            restart(screenWidth, screenHeight, &player, pickups, bodys, cutbodys,
                history, &historyIndex, &score, &wave, &pickupCount,
                &ticksAlive, &timeAlive, &ticksSinceShed, &timeSinceShed,
                &gameStart, &paused, &escPressedLastFrame,
                &startMenuTitleOffset, &startMenuTitleGrowing,
                &shedReady, &lost, &maxLength, &cutbodyIndex, enimies);
            gameStart = false;
            suppressSpaceToPlay = true;
        }

        // Check ESC once at the top
        if (IsKeyPressed(KEY_ESCAPE)) {
            paused = !paused; // toggle pause
        }

        if (player.size > maxLength) {
            maxLength = player.size;
        }

        if (IsKeyPressed(KEY_R)) {
            restart(screenWidth, screenHeight, &player, pickups, bodys, cutbodys,
        history, &historyIndex, &score, &wave, &pickupCount,
        &ticksAlive, &timeAlive, &ticksSinceShed, &timeSinceShed,
        &gameStart, &paused, &escPressedLastFrame,
        &startMenuTitleOffset, &startMenuTitleGrowing, &shedReady, &lost, &maxLength, &cutbodyIndex, enimies);
        }

        // Sound icon spin animation (only on start screen)
        if (!gameStart) {
            float dt = GetFrameTime();
            if (!soundSpinning) {
                soundWaitTimer -= dt;
                if (soundWaitTimer <= 0) {
                    soundSpinning = true;
                    soundSpinDuration = randomRange(0.5f, 2.0f);
                    soundSpinSpeed = randomRange(180.0f, 720.0f);
                }
            } else {
                soundSpinTimer += dt;
                soundRotation = sinf(soundSpinTimer * 5.0f) * 30.0f;
                if (soundSpinTimer >= soundSpinDuration) {
                    soundSpinning = false;
                    soundSpinTimer = 0.0f;
                    soundRotation = 0.0f;
                    soundWaitTimer = randomRange(1.0f, 4.0f);
                }
            }
        }

        BeginDrawing();

        if (gameStart) {
            if (!paused) {
                updateEntitiesAndValues(&player, &ticksSinceShed, &timeSinceShed, &shedReady,
                    gameWidth, gameHeight, margin, pickups, &pickupCount, &wave, &score,
                    bodys, &historyIndex, history, cutbodys, &cutbodyIndex, &paused, &lost, enimies, pickupWav, shedWav, &won, &speedBoost, &scoreMultiplier, &freeze);
                    
                updateClock(&ticksAlive, &timeAlive, &ticksSinceShed, &timeSinceShed, &fps, &shedReady, &player, speedBoost);
            }

            drawGameScreen(screenWidth, screenHeight, margin, gameWidth, gameHeight, cellSize,
                score, wave, timeAlive, timeSinceShed, shedReady, player, BACKGROUND,
                GRID_LINES, ACCENT, pickups, pickupCount, bodys, &historyIndex, history, cutbodys, enimies, speedBoost, scoreMultiplier, freeze);

            if (paused) {
                if (lost) {
                    drawRestartScreen(screenWidth, screenHeight, ACCENT, score, wave, timeAlive, player.size, maxLength);
                } else if (won) {
                    drawWinScreen(screenWidth, screenHeight, ACCENT, score, wave, timeAlive, player.size, maxLength);
                } else {
                    drawPausedScreen(screenWidth, screenHeight);
                }
            }

            UpdateMusicStream(gameMusic);

        } else {
            if(IsKeyPressed(KEY_RIGHT)) colorPickedIndex += 1;
            if(IsKeyPressed(KEY_LEFT)) colorPickedIndex -= 1;

            if (colorPickedIndex < 0) {
                colorPickedIndex = 5;
            }

            if (colorPickedIndex > 5) {
                colorPickedIndex = 0;
            }

            drawStartScreen(screenWidth, screenHeight, margin, gameWidth, gameHeight, &gameStart, BACKGROUND, ACCENT, &startMenuTitleOffset, &startMenuTitleGrowing, &suppressSpaceToPlay, soundImgTecture, soundRotation, &colorPickedIndex, &player);
            UpdateMusicStream(menuMusic);
        }

        EndDrawing();
    }

    UnloadTexture(soundImgTecture);
    UnloadImage(soundImg);

    UnloadMusicStream(menuMusic);
    UnloadMusicStream(gameMusic);
    UnloadSound(pickupWav);
    UnloadSound(shedWav);
    CloseAudioDevice();

    CloseWindow();
    
    return 0;
}

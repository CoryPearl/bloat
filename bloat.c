// Made by Cory Pearl
// Start date: 03/17/26
// Hours worked: 
// 17 coding 
// 4 desigining 
// 5 trying to complie to web asemmbley

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <raylib.h>

#define MAX_BODYS 99
#define MAX_CUT_BODYS 99
#define MAX_PICKUPS 100
#define MAX_ENIMIES 20
#define HISTORY_SIZE 1000
#define BASE_SPEED 125
#define BASE_TURN_SPEED 5

// TODO in order:s
// - If enimimie hits body enimie dies?
// - Win if get to wave 100

// Remember:
// - Jittering of player

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
    int size;
    bool shedReady;
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
} Pickup;

typedef struct {
    float x;
    float y;
    float speed;
    float r;
    float theta;
    int targetIndex;
    Color color;
} Enimie;

float randomRange(float n1, float n2) {
    return n1 + ((float)rand() / RAND_MAX) * (n2 - n1);
}

float distance(float x1, float y1, float x2, float y2) {
    return ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
}

void updateClock(float *ticksAlive, float *timeAlive, float *ticksSinceShed, float *timeSinceShed, float *fps, bool *shedReady, Player *player ) { // Updating clock
    *ticksAlive += 1;
    *timeAlive = *ticksAlive / *fps;

    if ( !*shedReady) {
        *ticksSinceShed += 1;
        *timeSinceShed = *ticksSinceShed / *fps;
    }

    if ( *timeSinceShed > 4) {
        player->speed = BASE_SPEED;
    }

    if ( *timeSinceShed > 4 && !*shedReady && player->size >= 6) {
        *shedReady = true;
        player->shedReady = true;
    }

}

bool checkCollision(float x1, float y1, float x2, float y2, int r1, int r2 ) {
    float dSquared = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
    float rSumSquared = (r1 + r2) * (r1 + r2);

    return dSquared <= rSumSquared;
}

void drawStartScreen(int screenWidth, int screenHeight, int margin, int gameWidth, int gameHeight, bool *gameStart, Color BACKGROUND, Color ACCENT, float *startMenuTitleOffset, bool *startMenuTitleGrowing, bool *suppressSpaceToPlay, Texture2D soundImg, float soundRotation) {
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

void drawGameScreen(int screenWidth, int screenHeight, int margin, int gameWidth, int gameHeight, int cellSize, int score, int wave, float timeAlive, float timeSinceShed, bool shedReady, Player player, Color BACKGROUND, Color GRID_LINES, Color ACCENT, Pickup *pickups, int pickupCount, Body *bodys, int *historyIndex, Vec2 *history, Cutbody *cutbodys, Enimie *enimies) {    
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
    DrawText("[ ESC ]  pause", gameWidth - MeasureText("[ ESC ] pause", 15) - 10, margin / 2 + 10, 15, DARKGRAY);

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
    DrawCircle(player.x + (cosf(player.theta) * 7.5), player.y + (sinf(player.theta) * 7.5), 5, BLACK);

    // Draw pickups 
    for (int i = 0; i < MAX_PICKUPS; i++) {
        if (pickups[i].x != 0 && pickups[i].y != 0) {
            DrawCircle(pickups[i].x, pickups[i].y, 5, ACCENT);
            DrawCircleLines(pickups[i].x, pickups[i].y, pickups[i].r, ACCENT);
        }
    }

    // Draw bodys
    for (int i = 0; i < MAX_BODYS; i++) {
        if (bodys[i].x != 0 && bodys[i].y != 0) {
            DrawCircle(bodys[i].x, bodys[i].y, bodys[i].r, bodys[i].color);
        }
    }

    // Draw cut bodys
    for (int i = 0; i < MAX_CUT_BODYS; i++) {
        if (cutbodys[i].x != 0 && cutbodys[i].y != 0) {
            DrawCircle(cutbodys[i].x, cutbodys[i].y, cutbodys[i].r, cutbodys[i].color);
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

void updateEntitiesAndValues(Player *player, float *ticksSinceShed, float *timeSinceShed, bool *shedReady, int gameWidth, int gameHeight, int margin, Pickup *pickups, int *pickupCount, int *wave, int *score, Body *bodys, int *historyIndex, Vec2 *history, Cutbody *cutbodys, int *cutbodyIndex, bool *paused, bool *lost, Enimie *enimies, Sound pickupWav, Sound shedWav) { 
    float dt = GetFrameTime();

    float currentTurnSpeed = BASE_TURN_SPEED / (1.0f + (player->size - 1) * 0.15f);

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
                *pickupCount -= 1;
                *score += player->size * 10;
                player->size += 1;
                // player->turnSpeed = BASE_TURN_SPEED - (player->size / 3);
                PlaySound(pickupWav);
            }
        }
    }

    // Check head collisions with bodys and with cut bodys

    if ( *pickupCount == 0) {
        *wave += 1;
        if (*wave == 100) {
            *paused = true;
        }
        *pickupCount = *wave;

        player->speed = BASE_SPEED + *wave;

        for (int i = 0; i < *wave; i++) {
            pickups[i].x = randomRange(17, gameWidth);
            pickups[i].y = randomRange(17, gameHeight - margin) + margin / 2;
            pickups[i].r = 5;
            pickups[i].growing = true;
        }

        for (int i = 0; i < MAX_ENIMIES; i++) {
            enimies[i].x = 0;
            enimies[i].y= 0;
        }

        for (int i = 0; i < randomRange(1, *wave); i++) {
            enimies[i].x = randomRange(17, gameWidth);
            enimies[i].y = randomRange(17, gameHeight - margin) + margin / 2;
            // enimies[i].r = 10;
            enimies[i].r = randomRange(4,14);
            enimies[i].speed = 100 - (enimies[i].r * 3);
            
        }
    }

    // Update positions of all bodys

    float baseSpacing = 17.5f;

    float currentSpacing = baseSpacing;

    float shrinkage = 1.01f;

    int offset = 0;

    float baseAlpha = 255.0f;
    float currentAlpha = baseAlpha;

    for (int i = 0; i < player->size - 1; i++) {

        offset += (int)currentSpacing;

        int index = *historyIndex - offset;
        if (index < 0) index += HISTORY_SIZE;

        bodys[i].x = history[index].x;
        bodys[i].y = history[index].y;

        if (i == 0) {
            bodys[i].r = 15 / shrinkage;
        } else {
            bodys[i].r = bodys[i - 1].r / shrinkage;
        }

        currentAlpha /= shrinkage;

        bodys[i].color = (Color){ 120, 199, 68, (unsigned char)currentAlpha };

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
    for (int i = 0; i < MAX_ENIMIES; i++) {
        if (enimies[i].x == 0 || enimies[i].y == 0) continue;

        float tx = player->x;
        float ty = player->y;
        float minDist = distance(enimies[i].x, enimies[i].y, tx, ty);
        int bestSeg = -1; /* -1 = targeting head */

        for (int j = 0; j < player->size - 1; j++) {
            float d = distance(enimies[i].x, enimies[i].y, bodys[j].x, bodys[j].y);
            if (d < minDist) {
                minDist = d;
                tx = bodys[j].x;
                ty = bodys[j].y;
                bestSeg = j;
            }
        }

        enimies[i].targetIndex = bestSeg;

        float dx = tx - enimies[i].x;
        float dy = ty - enimies[i].y;
        enimies[i].theta = atan2f(dy, dx);

        float evx = cosf(enimies[i].theta);
        float evy = sinf(enimies[i].theta);
        enimies[i].x += evx * enimies[i].speed * dt;
        enimies[i].y += evy * enimies[i].speed * dt;

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

    Player player;
    player.x = (float)screenWidth / 2;
    player.y = (float)screenHeight /2;
    player.theta = 0.0f;
    player.turnSpeed = BASE_TURN_SPEED; // Turn speed in degrees per second
    player.speed = BASE_SPEED; // Speed in pixels per second
    player.size = 1;
    player.shedReady = false;

    Pickup pickups[MAX_PICKUPS]; // Max pickups 100, (wave 100 you beat the game)

    for (int i = 0; i < MAX_PICKUPS; i++) {
        pickups[i].x = 0;
        pickups[i].y = 0;
        pickups[i].r = 5;
        pickups[i].growing = true;
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
                    bodys, &historyIndex, history, cutbodys, &cutbodyIndex, &paused, &lost, enimies, pickupWav, shedWav);
                    
                updateClock(&ticksAlive, &timeAlive, &ticksSinceShed, &timeSinceShed, &fps, &shedReady, &player);
            }

            drawGameScreen(screenWidth, screenHeight, margin, gameWidth, gameHeight, cellSize,
                score, wave, timeAlive, timeSinceShed, shedReady, player, BACKGROUND,
                GRID_LINES, ACCENT, pickups, pickupCount, bodys, &historyIndex, history, cutbodys, enimies);

            if (paused) {
                if (lost) {
                    drawRestartScreen(screenWidth, screenHeight, ACCENT, score, wave, timeAlive, player.size, maxLength);
                } else {
                    drawPausedScreen(screenWidth, screenHeight);
                }
            }

            UpdateMusicStream(gameMusic);

        } else {
            drawStartScreen(screenWidth, screenHeight, margin, gameWidth, gameHeight, &gameStart, BACKGROUND, ACCENT, &startMenuTitleOffset, &startMenuTitleGrowing, &suppressSpaceToPlay, soundImgTecture, soundRotation);
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
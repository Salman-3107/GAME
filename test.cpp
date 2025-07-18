#include "iGraphics.h"
#include "iSound.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <windows.h>

// Game states using enum
typedef enum
{
    STATE_MAIN_MENU,
    STATE_GAME,
    STATE_PLAYER_NAME_INPUT,
    STATE_INSTRUCTIONS,
    STATE_LEVEL_SELECT,
    STATE_SETTINGS,
    STATE_PAUSE,
    STATE_GAMEOVER,
    STATE_VICTORY,
    STATE_EXIT,
    STATE_GAME_OVER,
    STATE_LEADERBOARD
} GameState;

GameState currentState = STATE_MAIN_MENU;

int bgmChannel = -1;  // Track the background music channel
int soundEffectChannel = -1;  // Track sound effect channels

bool bgmInitialized = false;
bool bgmplaying = false;  
bool soundEnabled = true;
bool gameRunning = true;
//hover buttion
int hoveredButton = -1; 
//co-ordinate
int x,y;
// Ball
float ballX = 200, ballY = 300;
float ballRadius = 20;
float ballDY = 0;  
float ballDx = 0;
float gravity = -0.7;  
bool onGround = false;  
//sound button 
int soundOnBtnX = 300, soundOnBtnY = 350, soundBtnW = 150, soundBtnH = 50;
int soundOffBtnX = 500, soundOffBtnY = 350;


float vx = 0, vy = 0;
float moveSpeed = 7;
float jumpSpeed = 22; 

int score = 0; 
int currentLevel = 1;
//score
typedef struct {
    char name[50];
    int score;
    int level;
} HighScore;
//moving enemy 
typedef struct {
    int x, y;           // Grid position
    float offset;       // Current offset from grid position
    float speed;        // Movement speed
    int direction;      // 1 for right, -1 for left
    bool active;        // Whether enemy is active
    float timer;        // Individual timer for each enemy
} Enemy;

#define MAX_ENEMIES 50
Enemy enemies[MAX_ENEMIES];
int enemyCount = 0;
//compare
int compareScores(const void *a, const void *b) {
    HighScore *scoreA = (HighScore *)a;
    HighScore *scoreB = (HighScore *)b;
    return scoreB->score - scoreA->score;  // Descending order
}
// Player name system
char playerName[50] = ""; 
int nameLength = 0;
bool nameEntered = false;  // Flag to track 
bool isEnteringName = false;  // Flag for name input state

void gameOver();

// Map 
#define MAX_MAP_WIDTH 120
#define MAX_MAP_HEIGHT 8
char level1[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]= {
    "################################################################",
    "##...######.....P......###.........##..........................#",
    "##...######............###....P...........P....................#",
    "##...######...####.....###.........c...........................#",
    "##...######...####.....###..####..####..####......c....P.......#",
    "##............####..........##     ## ....##...................#",
    "##o......c....####.|....P...##...|....|...##.....|..|.....|...G#",
    "################################################################"
};
char level2[MAX_MAP_HEIGHT][MAX_MAP_WIDTH] = {
    "######################################################################################################################",
    "#.........................##......##..............................##.................##.........##...##..............#",
    "#......................P..##......##..............................##.................##.........##...................#",
    "#..............#########..##..##.........#####....................##........c........##.....................#........#",
    "#..............##.....##..##..##.........#####....................##.......##........##.........c....##...###........#",
    "#..............##.............##.........#####............................####..................##...##.....#........#",
    "#.xx....o....|.##..........c..##.........#####...xx....xx....xx....P.....######......P....xx....##...##.....#...xx..G#",
    "######################################################################################################################"
};

char level3[MAX_MAP_HEIGHT][MAX_MAP_WIDTH] = {
"######################################################################################",
"#......|...........|.........|....................|........|.........|............||.#",
"#....................................................................................#",
"#......c............................P....................c...........................#",
"#.....#............................###..............................................G#",
"#....###...........................###............xx.................................#",
"#|.#####..o...|...xxx.......|......###.....|||.....x...c..................P....xx...c#",
"######################################################################################"

};
//map backup
char level1_original[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
char level2_original[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
char level3_original[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
void initializeEnemies(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]);
void backupOriginalMaps() {
    memcpy(level1_original, level1, sizeof(level1));
    memcpy(level2_original, level2, sizeof(level2));
    memcpy(level3_original, level3, sizeof(level3));
}
// Camera position
float cameraX = 0, cameraY = 0;
int blockSize = 75;
int visibleWidth = 1000;
int visibleHeight = 600;

// Button variables for Main Menu
int btnX = 800, btnY = 100, btnW = 200, btnH = 50, gap = 20;
//
void restoreOriginalMaps() {
    memcpy(level1, level1_original, sizeof(level1));
    memcpy(level2, level2_original, sizeof(level2));
    memcpy(level3, level3_original, sizeof(level3));
}
//sound on off
void toggleSound(bool enable) {
    soundEnabled = enable;
    if (!enable) {
        // Stop all sounds when disabled
        iStopAllSounds();
        bgmChannel = -1;
        bgmplaying = false;
        bgmInitialized = false;
    } else {
        // When enabling sound, reset the initialization flag
        bgmInitialized = false;
    }
}
// Initialize game for new level
void initializeLevel() {
    ballX = 200;
    ballY = 300;
    ballDY = 0;
    ballDx = 0;
    vx = 0;
    vy = 0;
    onGround = false;
    cameraX = 0;
    cameraY = 0;
    if (currentLevel == 1) {
        initializeEnemies(level1);
    } else if (currentLevel == 2) {
        initializeEnemies(level2);
    } else if (currentLevel == 3) {
        initializeEnemies(level3);
    }
        restoreOriginalMaps();

}
//moving enemies
void initializeEnemies(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    enemyCount = 0;
    
    for (int y = 0; y < MAX_MAP_HEIGHT; y++) {
        for (int x = 0; x < MAX_MAP_WIDTH; x++) {
            if (map[y][x] == 'x') {
                if (enemyCount < MAX_ENEMIES) {
                    enemies[enemyCount].x = x;
                    enemies[enemyCount].y = y;
                    enemies[enemyCount].offset = 0;
                    // Give each enemy a slightly different speed for variety
                    enemies[enemyCount].speed = 1.0f + (rand() % 3) * 0.3f;
                    enemies[enemyCount].direction = (rand() % 2) ? 1 : -1;
                    enemies[enemyCount].active = true;
                    // Give each enemy a different starting timer
                    enemies[enemyCount].timer = rand() % 100;
                    enemyCount++;
                }
                // Remove 'x' from map since we're now tracking it separately
                map[y][x] = '.';
            }
        }
    }
}
//position for enemy 
void updateEnemies(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;
        
        // Update individual timer
        enemies[i].timer += 1.0f;
        
        // Move enemy based on its individual timer and speed
        enemies[i].offset += enemies[i].direction * enemies[i].speed * 0.02f;
        
        // Check boundaries and reverse direction
        // Move within a range of 3 blocks (1.5 blocks each side)
        if (enemies[i].offset > 1.5f) {
            enemies[i].offset = 1.5f;
            enemies[i].direction = -1;
        } else if (enemies[i].offset < -1.5f) {
            enemies[i].offset = -1.5f;
            enemies[i].direction = 1;
        }
        
        // Also check for wall collisions
        int checkX = enemies[i].x + (int)enemies[i].offset;
        if (checkX >= 0 && checkX < MAX_MAP_WIDTH && 
            enemies[i].y >= 0 && enemies[i].y < MAX_MAP_HEIGHT) {
            if (map[enemies[i].y][checkX] == '#') {
                enemies[i].direction = -enemies[i].direction;
            }
        }
    }
}
//drawing enemy
void drawEnemies() {
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;
        
        // Calculate screen position
        float worldX = (enemies[i].x + enemies[i].offset) * blockSize;
        float worldY = (MAX_MAP_HEIGHT - enemies[i].y - 1) * blockSize;
        
        int screenX = (int)(worldX - cameraX);
        int screenY = (int)(worldY - cameraY);
        
        // Only draw if on screen
        if (screenX > -blockSize && screenX < visibleWidth + blockSize &&
            screenY > -blockSize && screenY < visibleHeight + blockSize) {
            iShowImage(screenX, screenY, "assets/images/enemy.bmp");
        }
    }
}

// Initialize player name for new game session
void initializePlayerName() {
    memset(playerName, 0, sizeof(playerName));
    nameLength = 0;
    nameEntered = false;
    isEnteringName = false;
}

void displayLeaderboard() {
    FILE *file = fopen("highscore.txt", "r");
    if (file == NULL) {
        iSetColor(255, 255, 255);
        iText(400, 300, "No high scores yet!", GLUT_BITMAP_HELVETICA_18);
        iSetColor(0, 23, 66);
        iText(350, 30, "Press 'b' to go back to Main Menu", GLUT_BITMAP_HELVETICA_18);
        return;
    }

    HighScore scores[1000];  // Temporary array to store all scores
    int count = 0;
    char line[150];
    
    // Read all scores from file
    while (fgets(line, sizeof(line), file) && count < 1000) {
        char tempName[50];
        int tempLevel, tempScore;
        
        // Parse the line format: "Name - Level X: Score"
        if (sscanf(line, "%49s - Level %d: %d", tempName, &tempLevel, &tempScore) == 3) {
            // Check if this player already exists in our array
            int existingIndex = -1;
            for (int i = 0; i < count; i++) {
                if (strcmp(scores[i].name, tempName) == 0) {
                    existingIndex = i;
                    break;
                }
            }
            
            if (existingIndex != -1) {
                // Player exists, keep the higher score
                if (tempScore > scores[existingIndex].score) {
                    scores[existingIndex].level = tempLevel;
                    scores[existingIndex].score = tempScore;
                }
            } else {
                // New player, add to array
                strcpy(scores[count].name, tempName);
                scores[count].level = tempLevel;
                scores[count].score = tempScore;
                count++;
            }
        }
    }
    fclose(file);

    // Sort scores in descending order
    qsort(scores, count, sizeof(HighScore), compareScores);

    // Display header
    iSetColor(255, 255, 0);
    iText(400, 550, "TOP 10 HIGH SCORES", GLUT_BITMAP_TIMES_ROMAN_24);
    iSetColor(255, 255, 255);
    iText(200, 520, "Rank", GLUT_BITMAP_HELVETICA_18);
    iText(300, 520, "Name", GLUT_BITMAP_HELVETICA_18);
    iText(450, 520, "Level", GLUT_BITMAP_HELVETICA_18);
    iText(550, 520, "Score", GLUT_BITMAP_HELVETICA_18);

    // Draw a line under header
    iSetColor(255, 255, 255);
    iLine(180, 510, 620, 510);

    // Display top 10 scores
    int displayCount = (count < 10) ? count : 10;
    for (int i = 0; i < displayCount; i++) {
        char rankText[10];
        char levelText[10];
        char scoreText[20];
        
        sprintf(rankText, "%d", i + 1);
        sprintf(levelText, "%d", scores[i].level);
        sprintf(scoreText, "%d", scores[i].score);
        
        // Different colors for top 3
        if (i == 0) iSetColor(255, 215, 0);      // Gold
        else if (i == 1) iSetColor(192, 192, 192); // Silver
        else if (i == 2) iSetColor(205, 127, 50);  // Bronze
        else iSetColor(255, 255, 255);             // White
        
        int yPos = 480 - (i * 30);
        iText(210, yPos, rankText, GLUT_BITMAP_HELVETICA_18);
        iText(300, yPos, scores[i].name, GLUT_BITMAP_HELVETICA_18);
        iText(460, yPos, levelText, GLUT_BITMAP_HELVETICA_18);
        iText(550, yPos, scoreText, GLUT_BITMAP_HELVETICA_18);
    }

    // Display instructions
    iSetColor(0, 23, 66);
    iText(350, 40, "Press 'b' to go back to Main Menu", GLUT_BITMAP_HELVETICA_18);
}

void drawMap(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    
    // Draw normal map elements (excluding 'x' since we handle them separately)
    int startX = (int)(cameraX / blockSize) - 1;
    int endX = (int)((cameraX + visibleWidth) / blockSize) + 1;
    int startY = 0;
    int endY = MAX_MAP_HEIGHT;
    
    // Clamp to map bounds
    if(startX < 0) startX = 0;
    if(endX >= MAX_MAP_WIDTH) endX = MAX_MAP_WIDTH - 1;
    
    for (y = startY; y < endY; y++) {
        for (x = startX; x <= endX; x++) {
            char currentChar = map[y][x];
            int posX = (x * blockSize) - cameraX;
            int posY = ((MAX_MAP_HEIGHT - y - 1) * blockSize) - cameraY;

            // Skip if completely off screen
            if(posX < -blockSize || posX > visibleWidth || posY < -blockSize || posY > visibleHeight)
                continue;

            if (currentChar == '#') {
                iShowImage(posX, posY, "assets/images/block.bmp");
            } else if (currentChar == 'P') {
                iShowImage(posX, posY, "assets/images/ring-main.bmp");
            } else if (currentChar == 'c') {
                iShowImage(posX, posY, "assets/images/coin.bmp");
            } else if (currentChar == 'G') {
                iSetColor(0, 255, 0);
                iFilledCircle(posX + blockSize / 2, posY + blockSize / 2, blockSize / 2);
            }
            else if(currentChar == '|'){
                iShowImage(posX, posY, "assets/images/spike-main.bmp");
            }
            // Note: 'x' is handled separately in drawEnemies()
        }
    }
    
    // Draw enemies
    drawEnemies();
}
//collision with enemy 
void checkEnemyCollision() {
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;
        
        // Calculate enemy world position
        float enemyWorldX = (enemies[i].x + enemies[i].offset) * blockSize + blockSize / 2;
        float enemyWorldY = (MAX_MAP_HEIGHT - enemies[i].y - 1) * blockSize + blockSize / 2;
        
        // Calculate distance between ball and enemy
        float dx = ballX - enemyWorldX;
        float dy = ballY - enemyWorldY;
        float distance = sqrt(dx * dx + dy * dy);
        
        // Check if collision occurred (ball radius + enemy radius)
        if (distance < ballRadius + (blockSize / 2) * 0.8f) {
            // Game over - go back to main menu
          gameOver(); 
            return;
        }
    }
}

// Update camera position to ensure it stays within map bounds
void updateCameraPosition()
{
    float centerX = visibleWidth / 2.0f;
    float leftBound = centerX - 200;
    float rightBound = centerX + 200;
    
    float newCameraX = cameraX;

    if (ballX - cameraX < leftBound)
        newCameraX = ballX - leftBound;
    else if (ballX - cameraX > rightBound)
        newCameraX = ballX - rightBound;

    newCameraX = fmax(0, fmin(newCameraX, MAX_MAP_WIDTH * blockSize - visibleWidth));
    float newCameraY = fmax(0, fmin(cameraY, MAX_MAP_HEIGHT * blockSize - visibleHeight));
    
    cameraX = newCameraX;
    cameraY = newCameraY;
}

// Ensure the ball stays within the screen bounds and doesn't go out
void limitBallPosition() {
    if (ballX - ballRadius < cameraX) ballX = cameraX + ballRadius + 300;
    if (ballX + ballRadius > cameraX + visibleWidth) ballX = cameraX + visibleWidth - ballRadius - 300;
    if (ballY - ballRadius < cameraY) ballY = cameraY + ballRadius;
    if (ballY + ballRadius > cameraY + visibleHeight) ballY = cameraY + visibleHeight - ballRadius;
    if (ballY < ballRadius) {
        ballY = ballRadius;
        ballDY = 0;
        onGround = true;
    }
}

void checkSpikeCollision(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    int tileX = (int)(ballX / blockSize); 
    int tileYBelow = (int)((MAX_MAP_HEIGHT * blockSize - (ballY - ballRadius)) / blockSize);
    int tileYAbove = (int)((MAX_MAP_HEIGHT * blockSize - (ballY + ballRadius)) / blockSize);
    int tileXLeft = (int)((ballX - ballRadius) / blockSize);
    int tileXRight = (int)((ballX + ballRadius) / blockSize);

    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (map[tileYBelow][tileX] == '|') {
            gameOver();  
        }
    }

    if (tileYAbove >= 0 && tileYAbove < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (map[tileYAbove][tileX] == '|') {
            gameOver();  
        }
    }

    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXLeft >= 0 && tileXLeft < MAX_MAP_WIDTH) {
        if (map[tileYBelow][tileXLeft] == '|') {
            gameOver();  
        }
    }

    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXRight >= 0 && tileXRight < MAX_MAP_WIDTH) {
        if (map[tileYBelow][tileXRight] == '|') {
            gameOver();  
        }
    }
}

void checkVictory(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    int tileX = (int)(ballX / blockSize);
    int tileY = (int)((MAX_MAP_HEIGHT * blockSize - ballY) / blockSize);

    if (tileY >= 0 && tileY < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (map[tileY][tileX] == 'G') {
            currentState = STATE_VICTORY;
        }
    }
}

void collision(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]){
    // --- Vertical Collision Detection --- 
    int tileX = (int)(ballX / blockSize);
    int tileYBelow = (int)((MAX_MAP_HEIGHT * blockSize - (ballY - ballRadius)) / blockSize);
    int tileYAbove = (int)((MAX_MAP_HEIGHT * blockSize - (ballY + ballRadius)) / blockSize);

    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH)
    {
        if (map[tileYBelow][tileX] == '#')
        {
            ballY = (MAX_MAP_HEIGHT - tileYBelow) * blockSize + ballRadius;
            ballDY = -ballDY * 0.2f; 
            onGround = true;
        }
        else
        {
            onGround = false;
        }
    }

    if (tileYAbove >= 0 && tileYAbove < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH)
    {
        if (map[tileYAbove][tileX] == '#')
        {
            ballY = (MAX_MAP_HEIGHT - tileYAbove - 1) * blockSize - ballRadius;
            if (ballDY > 0)
                ballDY = 0;
        }
    }

    // --- Horizontal Collision Detection ---
    int tileYCenter = (int)((MAX_MAP_HEIGHT * blockSize - ballY) / blockSize);
    int tileXLeft = (int)((ballX - ballRadius) / blockSize);
    int tileXRight = (int)((ballX + ballRadius) / blockSize);

    if (tileYCenter >= 0 && tileYCenter < MAX_MAP_HEIGHT && tileXLeft >= 0 && tileXLeft < MAX_MAP_WIDTH)
    {
        if (map[tileYCenter][tileXLeft] == '#')
        {
            ballX = (tileXLeft + 1) * blockSize + ballRadius;
        }
    }

    if (tileYCenter >= 0 && tileYCenter < MAX_MAP_HEIGHT && tileXRight >= 0 && tileXRight < MAX_MAP_WIDTH)
    {
        if (map[tileYCenter][tileXRight] == '#')
        {
            ballX = tileXRight * blockSize - ballRadius;
        }
    }
}

void collection(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    int tileX = (int)(ballX / blockSize); 
    int tileY = (int)((MAX_MAP_HEIGHT * blockSize - (ballY)) / blockSize);

    if (tileY >= 0 && tileY < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (map[tileY][tileX] == 'c') { 
            score += 10;  
          if (soundEnabled) {  // play sound if enabled
                iPlaySound("assets/sounds/chime.wav", false); 
            }       
                map[tileY][tileX] = '.'; 
        }
    }
}

void updatePhysics() {
    if(currentState != STATE_GAME) return;
    
    // Get current map
    char (*currentMap)[MAX_MAP_WIDTH] = NULL;
    if (currentLevel == 1) currentMap = level1;
    else if (currentLevel == 2) currentMap = level2;
    else if (currentLevel == 3) currentMap = level3;
    
    if (!currentMap) return;
    
    // Apply movement
    if(isSpecialKeyPressed(GLUT_KEY_LEFT))
    {
        ballDx = -moveSpeed;
    }
    else if(isSpecialKeyPressed(GLUT_KEY_RIGHT))
    {
        ballDx = moveSpeed;
    }
    else
    {
        ballDx = 0;
    }
    
    if(isSpecialKeyPressed(GLUT_KEY_UP))
    {
        if (onGround)  
        {
            ballDY = 15;  
            onGround = false;  
        }
    }
    
    // Apply physics
    ballDY += gravity;
    ballY += ballDY;
    ballX += ballDx;

    if (ballY < ballRadius)
    {
        ballY = ballRadius;
        ballDY = 0;
        onGround = true;
    }

    // Check collisions and interactions
    collision(currentMap);
    checkSpikeCollision(currentMap);
    checkVictory(currentMap);
    collection(currentMap);
    updateEnemies(currentMap);
    limitBallPosition();
    updateCameraPosition();
    checkEnemyCollision();  

}

// Save player name and score to highscore.txt
void savePlayerScore(const char* name, int playerScore, int level) {
    FILE *file = fopen("highscore.txt", "a");
    if (file != NULL) {
        fprintf(file, "%s - Level %d: %d\n", name, level, playerScore);
        fclose(file);
    }
}

void gameOver() {
    if (nameEntered && strlen(playerName) > 0) {
        savePlayerScore(playerName, score, currentLevel);
    }
    currentState = STATE_GAME_OVER;
}

// Display high scores from file
void displayHighScore() {
    FILE *file = fopen("highscore.txt", "r");
    if (file != NULL) {
        char line[150];
        int highScore = 0;
        char highScorePlayer[50] = "";
        
        while (fgets(line, sizeof(line), file)) {
            char tempName[50];
            int tempLevel, tempScore;
            
            // format: "Name - Level X: Score"
            if (sscanf(line, "%49s - Level %d: %d", tempName, &tempLevel, &tempScore) == 3) {
                if (tempScore > highScore) {
                    highScore = tempScore;
                    strcpy(highScorePlayer, tempName);
                }
            }
        }
        fclose(file);

        if (highScore > 0) {
            char highScoreText[100];
            sprintf(highScoreText, "High Score: %s - %d", highScorePlayer, highScore);
            iSetColor(255, 255, 255);
            iText(20, 520, highScoreText, GLUT_BITMAP_HELVETICA_18);
        }
    }
}

void iDraw()
{
    iClear();

     if (soundEnabled && !bgmInitialized) {
        bgmChannel = iPlaySound("assets/sounds/game_audio.wav", true, 60);  // Loop background music
        if (bgmChannel != -1) {
            bgmplaying = true;
            bgmInitialized = true;
        }
    }
   if (currentState == STATE_MAIN_MENU) {
    iShowImage(0, 0, "assets/images/wallpaper.bmp");
   
    // Draw buttons based on hover state
    if (hoveredButton == 0) {
        iShowImage(btnX-15, btnY + 4 * (btnH + gap), "assets/images/start1.bmp");
    } else {
        iShowImage(btnX, btnY + 4 * (btnH + gap), "assets/images/start.bmp");
    }
    
    if (hoveredButton == 1) {
        iShowImage(btnX-15, btnY + 3 * (btnH + gap), "assets/images/instructions1.bmp");
    } else {
        iShowImage(btnX, btnY + 3 * (btnH + gap), "assets/images/instructions.bmp");
    }
    
    if (hoveredButton == 2) {
        iShowImage(btnX-15, btnY + (btnH + gap), "assets/images/settings1.bmp");
    } else {
        iShowImage(btnX, btnY + (btnH + gap), "assets/images/settings.bmp");
    }
    
    if (hoveredButton == 3) {
        iShowImage(btnX-15, btnY + 2 * (btnH + gap), "assets/images/leaderboad1.bmp");
    } else {
        iShowImage(btnX, btnY + 2 * (btnH + gap), "assets/images/leaderboad.bmp");
    }
    
    if (hoveredButton == 4) {
        iShowImage(btnX-15, btnY, "assets/images/exit1.bmp");
    } else {
        iShowImage(btnX, btnY, "assets/images/Exit.bmp");
    }
}
    else if (currentState == STATE_LEADERBOARD)
{
    iShowImage(0, 0, "assets/images/leaderboard_show.bmp"); 
    displayLeaderboard();
}
    else if (currentState == STATE_PLAYER_NAME_INPUT) {
        iShowImage(0, 0, "assets/images/name.bmp");
        
      
      
        iSetColor(0, 0, 0);
        // Draw player name with cursor
        char displayName[60];
        strcpy(displayName, playerName);
        if (isEnteringName) {
            strcat(displayName, "_");  // Show cursor
        }
        iText(310, 290, displayName, GLUT_BITMAP_HELVETICA_18);
    }
    else if (currentState == STATE_LEVEL_SELECT)
    {
        iShowImage(0, 0, "assets/images/level_select.bmp");
        iSetColor(255, 255, 255);  
        iText(320, 50, "Select a Level by pressing the number or by the mouse", GLUT_BITMAP_HELVETICA_18);
        
        // Show player name
        if (nameEntered && strlen(playerName) > 0) {
            char welcomeText[80];
            sprintf(welcomeText, "Welcome, %s!", playerName);
            iSetColor(255, 255, 0);
            iText(50, 550, welcomeText, GLUT_BITMAP_HELVETICA_18);
        }
    
// if(hoveredButton == 0) {
//             iShowImage(btnX-15, btnY + 3 * (btnH + gap), "assets/images/level1_1.bmp");
//         }   
//         else
//         {
//             iShowImage(btnX,btnY + 3*(btnH + gap), "assets/images/level1.bmp");
//         }
//  if(hoveredButton == 1) {
//             iShowImage(btnX-15, btnY + 2 * (btnH + gap), "assets/images/level2_1.bmp");
//         }
//         else
//         {
//             iShowImage(btnX, btnY + 2 * (btnH + gap), "assets/images/level2.bmp");
//         }

//  if(hoveredButton == 2) {

//             iShowImage(btnX-15, btnY + (btnH + gap), "assets/images/level3_1.bmp");
//         }
//         else
//         {
//             iShowImage(btnX, btnY + (btnH + gap), "assets/images/level3.bmp");
//         }









    }
    
    else if (currentState == STATE_GAME)
    {
        if (currentLevel == 1) {
            iShowImage(0, 0, "assets/images/level1.bmp");
            drawMap(level1);
        } else if (currentLevel == 2) {
            iShowImage(0, 0, "assets/images/level2.bmp");
            drawMap(level2);
        } else if (currentLevel == 3) {
            iShowImage(0, 0, "assets/images/level3.bmp");
            drawMap(level3);
        }
iShowSpeed(940,550);
        iSetColor(255, 0, 0);
        iFilledCircle(ballX - cameraX, ballY - cameraY, ballRadius);

        char scoreText[20];
        sprintf(scoreText, "Score: %d", score);
        iSetColor(255, 255, 255);
        iText(20, 560, scoreText, GLUT_BITMAP_HELVETICA_18);
        
        // Show player name during game
        if (nameEntered && strlen(playerName) > 0) {
            char playerText[60];
            sprintf(playerText, "Player: %s", playerName);
            iText(20, 530, playerText, GLUT_BITMAP_HELVETICA_12);
        }
    }
    else if (currentState == STATE_INSTRUCTIONS)
    {
        iShowImage(0, 0, "assets/images/instruction-main.bmp");
        iSetColor(123, 187, 23);
        // iFilledRectangle(120, 260, 700, 200);

        iSetColor(0, 0, 0);
        iText(120, 400, "Instructions:", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(120, 370, "- Use LEFT and RIGHT arrow keys to move the ball & SPACEBAR for jumping.", GLUT_BITMAP_HELVETICA_18);
        iText(120, 340, "- Beware of the spikes and butterflies.", GLUT_BITMAP_HELVETICA_18);
        iText(120, 310, "- The coins worth 10 points each,you have 3 lives to complete each level.", GLUT_BITMAP_HELVETICA_18);
        iText(120, 280, "- Press 'b' to return to the Main Menu.", GLUT_BITMAP_HELVETICA_18);
    }
   else if (currentState == STATE_SETTINGS)
    {
        iShowImage(0, 0, "assets/images/settings_menu.bmp");
        
        // Sound settings title
        iSetColor(0,26,66);
        iText(400, 450, "Sound Settings", GLUT_BITMAP_TIMES_ROMAN_24);
        
        // Sound ON button - improved visual feedback
        if (soundEnabled) {
            iSetColor(0, 200, 0);  // Bright green when active
            iFilledRectangle(soundOnBtnX, soundOnBtnY, soundBtnW, soundBtnH);
            iSetColor(255, 255, 255);  // White text for contrast
            iRectangle(soundOnBtnX, soundOnBtnY, soundBtnW, soundBtnH);  // Border
        } else {
            iSetColor(60, 60, 60);  // Dark gray when inactive
            iFilledRectangle(soundOnBtnX, soundOnBtnY, soundBtnW, soundBtnH);
            iSetColor(150, 150, 150);  // Light gray text
        }
        iText(soundOnBtnX + 45, soundOnBtnY + 20, "SOUND ON", GLUT_BITMAP_HELVETICA_18);
        
        // Sound OFF button - improved visual feedback
        if (!soundEnabled) {
            iSetColor(200, 0, 0);  // Bright red when active
            iFilledRectangle(soundOffBtnX, soundOffBtnY, soundBtnW, soundBtnH);
            iSetColor(255, 255, 255);  // White text for contrast
            iRectangle(soundOffBtnX, soundOffBtnY, soundBtnW, soundBtnH);  // Border
        } else {
            iSetColor(60, 60, 60);  // Dark gray when inactive
            iFilledRectangle(soundOffBtnX, soundOffBtnY, soundBtnW, soundBtnH);
            iSetColor(150, 150, 150);  // Light gray text
        }
        iText(soundOffBtnX + 40, soundOffBtnY + 20, "SOUND OFF", GLUT_BITMAP_HELVETICA_18);
        
        // Instructions
        iSetColor(0,26,66);
        iText(350, 200, "Click to toggle sound on/off", GLUT_BITMAP_HELVETICA_18);
        iText(350, 50, "Press 'b' to return to Main Menu", GLUT_BITMAP_HELVETICA_18);
        
        // Show current status
        iSetColor(255, 255, 0);
        char statusText[50];
        sprintf(statusText, "Current Status: %s", soundEnabled ? "Sound ON" : "Sound OFF");
        iText(350, 250, statusText, GLUT_BITMAP_HELVETICA_18);
    }
    else if (currentState == STATE_PAUSE)
    {
        if (currentLevel == 1) {
            iShowImage(0, 0, "assets/images/level1.bmp");
            drawMap(level1);
        } else if (currentLevel == 2) {
            iShowImage(0, 0, "assets/images/level2.bmp");
            drawMap(level2);
        } else if (currentLevel == 3) {
            iShowImage(0, 0, "assets/images/level3.bmp");
            drawMap(level3);
        }

        iSetColor(255, 0, 0);
        iFilledCircle(ballX - cameraX, ballY - cameraY, ballRadius);
        iShowImage(300, 170, "assets/images/pause.bmp");
    }
    else if (currentState == STATE_GAME_OVER)
    {
        iShowImage(0, 0, "assets/images/game-over.bmp");
        displayHighScore();
        
        // Show current player's score
        if (nameEntered && strlen(playerName) > 0) {
            char playerScoreText[100];
            sprintf(playerScoreText, "%s's Score: %d", playerName, score);
            iSetColor(255, 255, 0);
            iText(20, 480, playerScoreText, GLUT_BITMAP_HELVETICA_18);
        }
    }
    else if (currentState == STATE_VICTORY)
    {
        iShowImage(0, 0, "assets/images/victory.bmp");
        displayHighScore();
        
        // Show current player's score
        if (nameEntered && strlen(playerName) > 0) {
            char playerScoreText[100];
            sprintf(playerScoreText, "%s completed Level %d! Score: %d", playerName, currentLevel, score);
            iSetColor(255, 255, 0);
            iText(20, 480, playerScoreText, GLUT_BITMAP_HELVETICA_18);
        }
    }
}



void iKeyboard(unsigned char key, int state)
{
        if (state != GLUT_DOWN)  return;

    // Handle name input
    if (currentState == STATE_PLAYER_NAME_INPUT) {
        if (key == '\r' || key == '\n') { // Enter key
            if (nameLength > 0) {
                nameEntered = true;
                isEnteringName = false;
                currentState = STATE_LEVEL_SELECT;
            }
        }
        else if (key == '\b') { // Backspace
            if (nameLength > 0) {
                nameLength--;
                playerName[nameLength] = '\0';
            }
        }
        else if (key >= 32 && key <= 126 && nameLength < 49) { // Printable characters
            playerName[nameLength] = key;
            nameLength++;
            playerName[nameLength] = '\0';
        }
        return;
    }
     if (key == 'b' && currentState == STATE_LEADERBOARD) {
        currentState = STATE_MAIN_MENU;
    }
    if(key == 'b' && currentState ==STATE_LEVEL_SELECT){
        currentState = STATE_MAIN_MENU;
        initializeLevel();
    }

    if (key == 'b' && currentState == STATE_PAUSE) 
    {
        currentState = STATE_MAIN_MENU;
        initializeLevel();
    }
    if(key=='b' && currentState== STATE_INSTRUCTIONS){
        currentState = STATE_MAIN_MENU;
        initializeLevel();
    }
    if(key=='b' && currentState== STATE_SETTINGS){
        currentState = STATE_MAIN_MENU;
        initializeLevel();
    }
    if (key == ' ' && onGround && currentState == STATE_GAME)  
    {
        ballDY = jumpSpeed;
        onGround = false;
    }
    if (key == GLUT_KEY_UP && onGround && currentState == STATE_GAME)  
    {
        ballDY = 10;
        onGround = false;
    }

    if (currentState == STATE_GAME_OVER && key == 'b')  
    {
        currentState = STATE_MAIN_MENU;  
        initializeLevel(); 
        score = 0;
    }
    
    if (key == 'p' && currentState == STATE_GAME) 
    {
        currentState = STATE_PAUSE;
    }

    if (currentState == STATE_PAUSE && key == 'r')  
    {
        currentState = STATE_GAME;  
    }
    
    if (currentState == STATE_PAUSE)
    {
        if (key == 'r')
        {
            currentState = STATE_GAME;
        }
        else if (key == 'b')  
        {
            currentState = STATE_MAIN_MENU;
            initializeLevel();
        }
    }
    
    if (currentState == STATE_VICTORY && key == 'b') {
        currentState = STATE_LEVEL_SELECT;
        initializeLevel();
    }
    
    if (currentState == STATE_LEVEL_SELECT) 
    {
        if (key == '1') 
        {
            currentLevel = 1;
            initializeLevel();
            currentState = STATE_GAME;
        }
        else if (key == '2') 
        {
            currentLevel = 2;
            initializeLevel();
            currentState = STATE_GAME;
        }
        else if (key == '3')  
        {
            currentLevel = 3;
            initializeLevel();
            currentState = STATE_GAME;
        }
    }

    if (currentState == STATE_VICTORY && key == 'n') 
    {
        // Save score when level is completed
        if (nameEntered && strlen(playerName) > 0) {
            savePlayerScore(playerName, score, currentLevel);
        }
        
        if (currentLevel < 3) {
            currentLevel++;
            initializeLevel();
            currentState = STATE_GAME;
        } else {
            currentState = STATE_MAIN_MENU;
            initializeLevel();
        }
    }
}

void iMouseMove(int mx, int my) {
    if (currentState == STATE_MAIN_MENU) {
        if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 4 * (btnH + gap) && my <= btnY + 4 * (btnH + gap) + btnH) {
            hoveredButton = 0;  // Start button
        }
        else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 3 * (btnH + gap) && my <= btnY + 3 * (btnH + gap) + btnH) {
            hoveredButton = 1;  // Instructions button
        }
        else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + (btnH + gap) && my <= btnY + (btnH + gap) + btnH) {
            hoveredButton = 2;  // Settings button
        }
        else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 2 * (btnH + gap) && my <= btnY + 2 * (btnH + gap) + btnH) {
            hoveredButton = 3;  // Leaderboard button
        }
        else if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH) {
            hoveredButton = 4;  // Exit button
        }
        else {
            hoveredButton = -1;  // No button hovered
        }
    }

    else if(currentState == STATE_LEVEL_SELECT)
    {
        if (mx >= 50 && mx <= 320 && my >= 50 && my <= 300) {
            hoveredButton = 0;  // Level 1 button
        }
        else if (mx >= 350 && mx <= 620 && my >= 50 && my <= 300) {
            hoveredButton = 1;  // Level 2 button
        }
        else if (mx >= 650 && mx <= 920 && my >= 50 && my <= 300) {
            hoveredButton = 2;  // Level 3 button
        }}
        

    
    else {
        hoveredButton = -1;  // Reset when not in main menu
    }
}
void iMouseDrag(int mx, int my) {}
void iMouseWheel(int dir, int mx, int my) {}

void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (currentState == STATE_MAIN_MENU)
        {
            // Your existing main menu click handling...
            if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 4 * (btnH + gap) && my <= btnY + 4 * (btnH + gap) + btnH)
            {
                if (!nameEntered || strlen(playerName) == 0) {
                    currentState = STATE_PLAYER_NAME_INPUT;
                    isEnteringName = true;
                } else {
                    currentState = STATE_LEVEL_SELECT;
                }
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 3 * (btnH + gap) && my <= btnY + 3 * (btnH + gap) + btnH)
            {
                currentState = STATE_INSTRUCTIONS;
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + (btnH + gap) && my <= btnY + (btnH + gap) + btnH)
            {
               currentState = STATE_SETTINGS;
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 2 * (btnH + gap) && my <= btnY + 2 * (btnH + gap) + btnH)
            {
                currentState = STATE_LEADERBOARD; 
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH)
            {
                initializePlayerName();
                // Clean up sound before exiting
                iFreeSound();
                exit(0);
            }
        }
        else if (currentState == STATE_PLAYER_NAME_INPUT)
        {
            // Handle mouse clicks in name input state  
            if (mx >= 310 && mx <= 600 && my >= 290 && my <= 320) {
                isEnteringName = true;  // Clicked in the input area
            } else {
                isEnteringName = false;  // Clicked outside the input area
            }
        }
        else if (currentState == STATE_LEVEL_SELECT)
        {
            // Handle level selection
            if (mx >= 50 && mx <= 320 && my >= 50 && my <= 300) {
                currentLevel = 1;
                initializeLevel();
                currentState = STATE_GAME;
            } else if (mx >= 350 && mx <= 620 && my >= 50 && my <= 300) {
                currentLevel = 2;
                initializeLevel();
                currentState = STATE_GAME;
            } else if (mx >= 650 && mx <= 920 && my >= 50 && my <= 300) {
                currentLevel = 3;
                initializeLevel();
                currentState = STATE_GAME;
            }
        }


        else if (currentState == STATE_SETTINGS)
        {
            // Sound ON button clicked
            if (mx >= soundOnBtnX && mx <= soundOnBtnX + soundBtnW && 
                my >= soundOnBtnY && my <= soundOnBtnY + soundBtnH)
            {
                if (!soundEnabled) {  // Only toggle if currently off
                    toggleSound(true);
                }
            }
            // Sound OFF button clicked
            else if (mx >= soundOffBtnX && mx <= soundOffBtnX + soundBtnW && 
                     my >= soundOffBtnY && my <= soundOffBtnY + soundBtnH)
            {
                if (soundEnabled) {  // Only toggle if currently on
                    toggleSound(false);
                }
            }
        }
    }
}

void iMovement(int value)
{
    updatePhysics();
    glutTimerFunc(16, iMovement, 0);
}

void iSpecialKeyboard(unsigned char key, int state){}

void iSpecialKeyboardUp(unsigned char key,int state)
{
    if (key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT)
    {
        ballDx = 0;  
    }
}

int main(int argc, char *argv[])
{
    printf("Starting game...\n");
    
    // Initialize player name
    playerName[0] = '\0';
    backupOriginalMaps();

    glutInit(&argc, argv);
    iInitializeSound();
    glutTimerFunc(16, iMovement, 0);
    iOpenWindow(1000, 600, "Bounce Classic");
    atexit(iFreeSound);

    return 0;
}

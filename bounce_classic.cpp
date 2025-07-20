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
    STATE_ABOUT_US,
    STATE_EXIT,
    STATE_GAME_OVER,
    STATE_LEADERBOARD
} GameState;
// ball rotation
float ballRotation = 0.0f;
GameState currentState = STATE_MAIN_MENU;
int lives = 3;
int bgmChannel = -1;         // Track the background music channel
int soundEffectChannel = -1; // Track sound effect channels
// checkpoint
typedef struct
{
    float x, y;
    bool activated;
} Checkpoint;
#define MAX_CHECKPOINTS 10
Checkpoint checkpoints[MAX_CHECKPOINTS];
int checkpointCount = 0;
int lastCheckpointIndex = -1;                       // Index of last activated checkpoint
float lastCheckpointX = 200, lastCheckpointY = 300; // Default spawn position

bool bgmInitialized = false;
bool bgmplaying = false;
bool soundEnabled = true;
bool gameRunning = true;
// hover buttion
int hoveredButton = -1;
// co-ordinate
int x, y;
// Ball
float ballX = 200, ballY = 300;
float ballRadius = 20;
float ballDY = 0;
float ballDx = 0;
float gravity = -0.7;
bool onGround = false;
// sound button
int soundOnBtnX = 300, soundOnBtnY = 350, soundBtnW = 150, soundBtnH = 50;
int soundOffBtnX = 500, soundOffBtnY = 350;

float vx = 0, vy = 0;
float moveSpeed = 7;
float jumpSpeed = 22;

int score = 0;
int currentLevel = 1;
// score
typedef struct
{
    char name[50];
    int score;
    int level;
} HighScore;
// moving enemy
typedef struct
{
    int x, y;      // Grid position
    float offset;  // Current offset from grid position
    float speed;   // Movement speed
    int direction; // 1 for right, -1 for left
    bool active;   // Whether enemy is active
    float timer;   // Individual timer for each enemy
} Enemy;

#define MAX_ENEMIES 50
Enemy enemies[MAX_ENEMIES];
int enemyCount = 0;
// compare
int compareScores(const void *a, const void *b)
{
    HighScore *scoreA = (HighScore *)a;
    HighScore *scoreB = (HighScore *)b;
    return scoreB->score - scoreA->score; // Descending order
}
// Player name system
char playerName[50] = "";
int nameLength = 0;
bool nameEntered = false;    // Flag to track
bool isEnteringName = false; // Flag for name input state

void gameOver();

// Map
#define MAX_MAP_WIDTH 120
#define MAX_MAP_HEIGHT 8
char level1[MAX_MAP_HEIGHT][MAX_MAP_WIDTH] = {
    "#######################################################################################################################",
    "##...######.....P......###.........##.................................................|...............................#",
    "##...######............###....P...........P...........................................c...............................#",
    "##...######...####.....###.........c..................................................#...............................#",
    "##...######...####.....###..####..####..####......c....P............c.................##..............................#",
    "##............####..........##     ## ....##.........................................####.............................#",
    "##o......c....####.|....P...##...|....|...##.....|..|.....|.....xx...xx...c..|..c...######....P..xx......|..c...|.....G",
    "#######################################################################################################################"};
char level2[MAX_MAP_HEIGHT][MAX_MAP_WIDTH] = {
    "#######################################################################################################################",
    "#.........................##......##..............................##.................##.........##...##...............#",
    "#......................P..##......##..............................##.................##.........##....................#",
    "#..............#########..##..##.........#####....................##........c........##.....................#.........#",
    "#..............##.....##..##..##.........#####....................##.......##........##.........c....##...###.........#",
    "#..............##.............##.........#####............................####..................##...##.....#.........#",
    "#.......xx...|.##..........c..##.........#####...xx....xx....xx....P.....######......P....xx....##...##.....#...xx....G",
    "#######################################################################################################################"};

char level3[MAX_MAP_HEIGHT][MAX_MAP_WIDTH] = {
    "#######################################################################################################################",
    "#......|...........|.........|....................|........|.........|............||..................................#",
    "#...........................................................................................c.........................#",
    "#......c............................P....................c.........#####..............###.......x.....................#",
    "#.....#............................###.............................#####..............#####..####.....................G",
    "#....###...........................###............xx...............#####..............#.........#.....................#",
    "#|.#####..o...|...xxx.......|......###.....|||.....x...c...........#####..P....xx.....#.....|...#....|..c..|.xx.......#",
    "#######################################################################################################################"

};

// ini
void initializeCheckpoints(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]);
void checkCheckpointCollision(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]);
// map backup
char level1_original[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
char level2_original[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
char level3_original[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
void initializeEnemies(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]);
void backupOriginalMaps()
{
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
void restoreOriginalMaps()
{
    memcpy(level1, level1_original, sizeof(level1));
    memcpy(level2, level2_original, sizeof(level2));
    memcpy(level3, level3_original, sizeof(level3));
}
// sound on off
void toggleSound(bool enable)
{
    soundEnabled = enable;
    if (!enable)
    {
        // Stop all sounds when disabled
        iStopAllSounds();
        bgmChannel = -1;
        bgmplaying = false;
        bgmInitialized = false;
    }
    else
    {
        // When enabling sound, reset the initialization flag
        bgmInitialized = false;
    }
}
// Initialize game for new level
void initializeLevel()
{
    lives = 3; 
    ballX = 200;
    ballY = 300;
    ballDY = 0;
    ballDx = 0;
    vx = 0;
    vy = 0;
    score = 0;
    ballRotation = 0.0f;

    onGround = false;
    cameraX = 0;
    cameraY = 0;

    // Initialize checkpoints for current level
    if (currentLevel == 1)
    {
        initializeEnemies(level1);
        initializeCheckpoints(level1);
    }
    else if (currentLevel == 2)
    {
        initializeEnemies(level2);
        initializeCheckpoints(level2);
    }
    else if (currentLevel == 3)
    {
        initializeEnemies(level3);
        initializeCheckpoints(level3);
    }

    restoreOriginalMaps();
}
// checkpoint indicate
void initializeCheckpoints(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH])
{
    checkpointCount = 0;
    lastCheckpointIndex = -1;
    lastCheckpointX = 200;
    lastCheckpointY = 300;

    for (int y = 0; y < MAX_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_MAP_WIDTH; x++)
        {
            if (map[y][x] == 'P')
            {
                if (checkpointCount < MAX_CHECKPOINTS)
                {
                    checkpoints[checkpointCount].x = x * blockSize + blockSize / 2;
                    checkpoints[checkpointCount].y = (MAX_MAP_HEIGHT - y - 1) * blockSize + blockSize / 2;
                    checkpoints[checkpointCount].activated = false;
                    checkpointCount++;
                }
            }
        }
    }
}
// checkpoint collision hit
void checkCheckpointCollision(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH])
{
    for (int i = 0; i < checkpointCount; i++)
    {
        if (!checkpoints[i].activated)
        {
            float dx = ballX - checkpoints[i].x;
            float dy = ballY - checkpoints[i].y;
            float distance = sqrt(dx * dx + dy * dy);

            // Check if ball is close enough to checkpoint
            if (distance < ballRadius + (blockSize / 2) * 0.8f)
            {
                checkpoints[i].activated = true;
                lastCheckpointIndex = i;
                lastCheckpointX = checkpoints[i].x;
                lastCheckpointY = checkpoints[i].y;

                // Update the map to show activated checkpoint
                int mapX = (int)(checkpoints[i].x / blockSize);
                int mapY = (int)((MAX_MAP_HEIGHT * blockSize - checkpoints[i].y) / blockSize);
                if (mapX >= 0 && mapX < MAX_MAP_WIDTH && mapY >= 0 && mapY < MAX_MAP_HEIGHT)
                {
                    map[mapY][mapX] = 'Q'; // Use 'Q' to represent activated checkpoint
                }

                // Play checkpoint sound if enabled
                if (soundEnabled)
                {
                    iPlaySound("assets/sounds/chime.wav", false);
                }
                break;
            }
        }
    }
}
// respawn
void respawnAtCheckpoint()
{
    ballX = lastCheckpointX;
    ballY = lastCheckpointY;
    ballDY = 0;
    ballDx = 0;
    vx = 0;
    vy = 0;
    ballRotation = 0.0f;
    onGround = false;

    // Update camera to follow ball
    cameraX = ballX - visibleWidth / 2;
    cameraY = 0;
    if (cameraX < 0)
        cameraX = 0;
    if (cameraX > MAX_MAP_WIDTH * blockSize - visibleWidth)
    {
        cameraX = MAX_MAP_WIDTH * blockSize - visibleWidth;
    }
}

// moving enemies
void initializeEnemies(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH])
{
    enemyCount = 0;

    for (int y = 0; y < MAX_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_MAP_WIDTH; x++)
        {
            if (map[y][x] == 'x')
            {
                if (enemyCount < MAX_ENEMIES)
                {
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
// position for enemy
void updateEnemies(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH])
{
    for (int i = 0; i < enemyCount; i++)
    {
        if (!enemies[i].active)
            continue;

        // Update individual timer
        enemies[i].timer += 1.0f;

        // Move enemy based on its individual timer and speed
        enemies[i].offset += enemies[i].direction * enemies[i].speed * 0.02f;

        // Check boundaries and reverse direction
        // Move within a range of 3 blocks (1.5 blocks each side)
        if (enemies[i].offset > 1.5f)
        {
            enemies[i].offset = 1.5f;
            enemies[i].direction = -1;
        }
        else if (enemies[i].offset < -1.5f)
        {
            enemies[i].offset = -1.5f;
            enemies[i].direction = 1;
        }

        // Also check for wall collisions
        int checkX = enemies[i].x + (int)enemies[i].offset;
        if (checkX >= 0 && checkX < MAX_MAP_WIDTH &&
            enemies[i].y >= 0 && enemies[i].y < MAX_MAP_HEIGHT)
        {
            if (map[enemies[i].y][checkX] == '#')
            {
                enemies[i].direction = -enemies[i].direction;
            }
        }
    }
}
// drawing enemy
void drawEnemies()
{
    for (int i = 0; i < enemyCount; i++)
    {
        if (!enemies[i].active)
            continue;

        // Calculate screen position
        float worldX = (enemies[i].x + enemies[i].offset) * blockSize;
        float worldY = (MAX_MAP_HEIGHT - enemies[i].y - 1) * blockSize;

        int screenX = (int)(worldX - cameraX);
        int screenY = (int)(worldY - cameraY);

        // Only draw if on screen
        if (screenX > -blockSize && screenX < visibleWidth + blockSize &&
            screenY > -blockSize && screenY < visibleHeight + blockSize)
        {
            iShowImage(screenX, screenY, "assets/images/enemy.bmp");
        }
    }
}

// Initialize player name for new game session
void initializePlayerName()
{
    memset(playerName, 0, sizeof(playerName));
    nameLength = 0;
    nameEntered = false;
    isEnteringName = false;
}

void displayLeaderboard()
{
    FILE *file = fopen("highscore.txt", "r");
    if (file == NULL)
    {
        iSetColor(255, 255, 255);
        iText(400, 300, "No high scores yet!", GLUT_BITMAP_HELVETICA_18);
        iSetColor(0, 23, 66);
        iText(350, 30, "Press 'b' to go back to Main Menu", GLUT_BITMAP_HELVETICA_18);
        return;
    }

    HighScore scores[1000]; // Temporary array to store all scores
    int count = 0;
    char line[150];

    // Read all scores from file
    while (fgets(line, sizeof(line), file) && count < 1000)
    {
        char tempName[50];
        int tempLevel, tempScore;

        // Parse the line format: "Name - Level X: Score"
        if (sscanf(line, "%49s - Level %d: %d", tempName, &tempLevel, &tempScore) == 3)
        {
            // Check if this player already exists in our array
            int existingIndex = -1;
            for (int i = 0; i < count; i++)
            {
                if (strcmp(scores[i].name, tempName) == 0)
                {
                    existingIndex = i;
                    break;
                }
            }

            if (existingIndex != -1)
            {
                // Player exists, keep the higher score
                if (tempScore > scores[existingIndex].score)
                {
                    scores[existingIndex].level = tempLevel;
                    scores[existingIndex].score = tempScore;
                }
            }
            else
            {
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
    for (int i = 0; i < displayCount; i++)
    {
        char rankText[10];
        char levelText[10];
        char scoreText[20];

        sprintf(rankText, "%d", i + 1);
        sprintf(levelText, "%d", scores[i].level);
        sprintf(scoreText, "%d", scores[i].score);

        // Different colors for top 3
        if (i == 0)
            iSetColor(255, 215, 0); // Gold
        else if (i == 1)
            iSetColor(192, 192, 192); // Silver
        else if (i == 2)
            iSetColor(205, 127, 50); // Bronze
        else
            iSetColor(0, 0,0); 

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

void drawMap(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH])
{
    int startX = (int)(cameraX / blockSize) - 1;
    int endX = (int)((cameraX + visibleWidth) / blockSize) + 1;
    int startY = 0;
    int endY = MAX_MAP_HEIGHT;

    if (startX < 0)
        startX = 0;
    if (endX >= MAX_MAP_WIDTH)
        endX = MAX_MAP_WIDTH - 1;

    for (int y = startY; y < endY; y++)
    {
        for (int x = startX; x <= endX; x++)
        {
            char currentChar = map[y][x];
            int posX = (x * blockSize) - cameraX;
            int posY = ((MAX_MAP_HEIGHT - y - 1) * blockSize) - cameraY;

            if (posX < -blockSize || posX > visibleWidth || posY < -blockSize || posY > visibleHeight)
                continue;

            if (currentChar == '#')
            {
                iShowImage(posX, posY, "assets/images/block.bmp");
            }
            else if (currentChar == 'P')
            {
                iShowImage(posX, posY, "assets/images/ring-main.bmp");
            }
            else if (currentChar == 'Q')
            { // Activated checkpoint
                iShowImage(posX, posY, "assets/images/ring2.bmp");
            }
            else if (currentChar == 'c')
            {
                iShowImage(posX, posY, "assets/images/coin.bmp");
            }
            else if (currentChar == 'G')
            {
                iShowImage(posX, posY, "assets/images/victory_point.bmp");
            }
            else if (currentChar == '|')
            {
                iShowImage(posX, posY, "assets/images/spike-main.bmp");
            }
        }
    }

    drawEnemies();
}
// collision with enemy
void checkEnemyCollision()
{
    for (int i = 0; i < enemyCount; i++)
    {
        if (!enemies[i].active)
            continue;

        float enemyWorldX = (enemies[i].x + enemies[i].offset) * blockSize + blockSize / 2;
        float enemyWorldY = (MAX_MAP_HEIGHT - enemies[i].y - 1) * blockSize + blockSize / 2;

        float dx = ballX - enemyWorldX;
        float dy = ballY - enemyWorldY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < ballRadius + (blockSize / 2) * 0.8f)
        {
            lives--;
            if (lives <= 0)
            {
                currentState = STATE_GAME_OVER;
            }
            else
            {
                respawnAtCheckpoint(); // Respawn
            }
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

    // Find the rightmost non-empty block in the current level
    int rightmostBlock = 0;
    for (int y = 0; y < MAX_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAX_MAP_WIDTH; x++)
        {
            if (level1[y][x] != ' ' || level2[y][x] != ' ' || level3[y][x] != ' ')
            {
                if (x > rightmostBlock)
                {
                    rightmostBlock = x;
                }
            }
        }
    }

    // Calculate the maximum camera position
    // The camera should stop when the rightmost content is at the right edge of screen
    float maxCameraX = (rightmostBlock + 1) * blockSize - visibleWidth;

    // Don't allow negative camera positions
    if (maxCameraX < 0)
    {
        maxCameraX = 0;
    }

    // Update camera's X position based on ball's movement
    float newCameraX = cameraX;

    if (ballX - cameraX < leftBound)
        newCameraX = ballX - leftBound;
    else if (ballX - cameraX > rightBound)
        newCameraX = ballX - rightBound;

    // Ensure camera doesn't exceed the maximum position
    newCameraX = fmax(0, fmin(newCameraX, maxCameraX));

    // Update camera Y position
    float newCameraY = fmax(0, fmin(cameraY, MAX_MAP_HEIGHT * blockSize - visibleHeight));

    // Apply the updated camera position
    cameraX = newCameraX;
    cameraY = newCameraY;
}

// Ensure the ball stays within the screen bounds and doesn't go out
void limitBallPosition()
{
    if (ballX - ballRadius < cameraX)
        ballX = cameraX + ballRadius + 300;
    if (ballX + ballRadius > cameraX + visibleWidth)
        ballX = cameraX + visibleWidth - ballRadius - 300;
    if (ballY - ballRadius < cameraY)
        ballY = cameraY + ballRadius;
    if (ballY + ballRadius > cameraY + visibleHeight)
        ballY = cameraY + visibleHeight - ballRadius;
    if (ballY < ballRadius)
    {
        ballY = ballRadius;
        ballDY = 0;
        onGround = true;
    }
}

void checkSpikeCollision(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH])
{
    int tileX = (int)(ballX / blockSize);
    int tileYBelow = (int)((MAX_MAP_HEIGHT * blockSize - (ballY - ballRadius)) / blockSize);
    int tileYAbove = (int)((MAX_MAP_HEIGHT * blockSize - (ballY + ballRadius)) / blockSize);
    int tileXLeft = (int)((ballX - ballRadius) / blockSize);
    int tileXRight = (int)((ballX + ballRadius) / blockSize);

    bool hitSpike = false;

    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH)
    {
        if (map[tileYBelow][tileX] == '|')
        {
            hitSpike = true;
        }
    }

    if (tileYAbove >= 0 && tileYAbove < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH)
    {
        if (map[tileYAbove][tileX] == '|')
        {
            hitSpike = true;
        }
    }

    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXLeft >= 0 && tileXLeft < MAX_MAP_WIDTH)
    {
        if (map[tileYBelow][tileXLeft] == '|')
        {
            hitSpike = true;
        }
    }

    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXRight >= 0 && tileXRight < MAX_MAP_WIDTH)
    {
        if (map[tileYBelow][tileXRight] == '|')
        {
            hitSpike = true;
        }
    }

    if (hitSpike)
    {
        lives--;
        if (lives <= 0)
        {
            currentState = STATE_GAME_OVER;
        }
        else
        {
            respawnAtCheckpoint(); // Respawn at last checkpoint instead of level start
        }
    }
}

void checkVictory(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH])
{
    int tileX = (int)(ballX / blockSize);
    int tileY = (int)((MAX_MAP_HEIGHT * blockSize - ballY) / blockSize);

    if (tileY >= 0 && tileY < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH)
    {
        if (map[tileY][tileX] == 'G')
        {
            currentState = STATE_VICTORY;
        }
    }
}

void collision(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH])
{
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

void collection(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH])
{
    int tileX = (int)(ballX / blockSize);
    int tileY = (int)((MAX_MAP_HEIGHT * blockSize - (ballY)) / blockSize);

    if (tileY >= 0 && tileY < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH)
    {
        if (map[tileY][tileX] == 'c')
        {
            score += 10;
            if (soundEnabled)
            { // play sound if enabled
                iPlaySound("assets/sounds/chime.wav", false);
            }
            map[tileY][tileX] = '.';
        }
    }
}

void updatePhysics()
{
    if (currentState != STATE_GAME)
        return;

    char (*currentMap)[MAX_MAP_WIDTH] = NULL;
    if (currentLevel == 1)
        currentMap = level1;
    else if (currentLevel == 2)
        currentMap = level2;
    else if (currentLevel == 3)
        currentMap = level3;

    if (!currentMap)
        return;

    // Apply movement
    if (isSpecialKeyPressed(GLUT_KEY_LEFT))
    {
        ballDx = -moveSpeed;
        ballRotation += 5.0f;
        if (ballRotation < 0)
            ballRotation += 360.0f;
    }
    else if (isSpecialKeyPressed(GLUT_KEY_RIGHT))
    {
        ballDx = moveSpeed;
        ballRotation -= 5.0f;
        if (ballRotation >= 360.0f)
            ballRotation -= 360.0f;
    }
    else
    {
        ballDx = 0;
    }

    if (isSpecialKeyPressed(GLUT_KEY_UP))
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
    checkCheckpointCollision(currentMap); // Add checkpoint checking
    updateEnemies(currentMap);
    limitBallPosition();
    updateCameraPosition();
    checkEnemyCollision();
}
// rotate func
void drawRotatedBall(int x, int y, const char *imagePath, float angle)
{
    // Save current transformation matrix
    glPushMatrix();

    // Translate to ball position
    glTranslatef(x + ballRadius, y + ballRadius, 0);

    // Rotate around the center
    glRotatef(angle, 0, 0, 1);

    // Draw the image centered at origin
    iShowImage(-ballRadius, -ballRadius, imagePath);

    // Restore transformation matrix
    glPopMatrix();
}

// Save player name and score to highscore.txt
void savePlayerScore(const char *name, int playerScore, int level)
{
    FILE *file = fopen("highscore.txt", "a");
    if (file != NULL)
    {
        fprintf(file, "%s - Level %d: %d\n", name, level, playerScore);
        fclose(file);
    }
}

void gameOver()
{
    if (nameEntered && strlen(playerName) > 0)
    {
        savePlayerScore(playerName, score, currentLevel);
    }
    currentState = STATE_GAME_OVER;
}

// Display high scores from file
void displayHighScore()
{
    FILE *file = fopen("highscore.txt", "r");
    if (file != NULL)
    {
        char line[150];
        int highScore = 0;
        char highScorePlayer[50] = "";

        while (fgets(line, sizeof(line), file))
        {
            char tempName[50];
            int tempLevel, tempScore;

            // format: "Name - Level X: Score"
            if (sscanf(line, "%49s - Level %d: %d", tempName, &tempLevel, &tempScore) == 3)
            {
                if (tempScore > highScore)
                {
                    highScore = tempScore;
                    strcpy(highScorePlayer, tempName);
                }
            }
        }
        fclose(file);

        if (highScore > 0)
        {
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

    if (soundEnabled && !bgmInitialized)
    {
        bgmChannel = iPlaySound("assets/sounds/game_audio.wav", true, 60); // Loop background music
        if (bgmChannel != -1)
        {
            bgmplaying = true;
            bgmInitialized = true;
        }
    }
    if (currentState == STATE_MAIN_MENU)
    {
        iShowImage(0, 0, "assets/images/wallpaper.bmp");

        // Draw buttons based on hover state
        if (hoveredButton == 0)
        {
            iShowImage(btnX - 15, btnY + 5 * (btnH + gap), "assets/images/start1.bmp");
        }
        else
        {
            iShowImage(btnX, btnY + 5 * (btnH + gap), "assets/images/start.bmp");
        }

        if (hoveredButton == 1)
        {
            iShowImage(btnX - 15, btnY + 4 * (btnH + gap), "assets/images/instructions1.bmp");
        }
        else
        {
            iShowImage(btnX, btnY + 4 * (btnH + gap), "assets/images/instructions.bmp");
        }

        if (hoveredButton == 2)
        {
            iShowImage(btnX - 15, btnY + 2 * (btnH + gap), "assets/images/settings1.bmp");
        }
        else
        {
            iShowImage(btnX, btnY + 2 * (btnH + gap), "assets/images/settings.bmp");
        }

        if (hoveredButton == 3)
        {
            iShowImage(btnX - 15, btnY + 3 * (btnH + gap), "assets/images/leaderboad1.bmp");
        }
        else
        {
            iShowImage(btnX, btnY + 3 * (btnH + gap), "assets/images/leaderboad.bmp");
        }

        if (hoveredButton == 4)
        {
            iShowImage(btnX - 15, btnY, "assets/images/exit1.bmp");
        }
        else
        {
            iShowImage(btnX, btnY, "assets/images/Exit.bmp");
        }

        if (hoveredButton == 5)
        {
            iShowImage(btnX - 15, btnY + 1 * (btnH + gap), "assets/images/about_us1.bmp");
        }
        else
        {
            iShowImage(btnX, btnY + 1 * (btnH + gap), "assets/images/about_us.bmp");
        }
    }
    else if (currentState == STATE_LEADERBOARD)
    {
        iShowImage(0, 0, "assets/images/leaderboard_show.bmp");
        displayLeaderboard();
    }
    else if (currentState == STATE_PLAYER_NAME_INPUT)
    {
        iShowImage(0, 0, "assets/images/name.bmp");

        iSetColor(0, 0, 0);
        // Draw player name with cursor
        char displayName[60];
        strcpy(displayName, playerName);
        if (isEnteringName)
        {
            strcat(displayName, "_"); // Show cursor
        }
        iText(310, 290, displayName, GLUT_BITMAP_HELVETICA_18);
    }
    else if (currentState == STATE_LEVEL_SELECT)
    {
        iShowImage(0, 0, "assets/images/level_select.bmp");
        iSetColor(255, 255, 255);
        iText(320, 50, "Select a Level by pressing the number or by the mouse", GLUT_BITMAP_HELVETICA_18);

        // Show player name
        if (nameEntered && strlen(playerName) > 0)
        {
            char welcomeText[80];
            sprintf(welcomeText, "Welcome, %s!", playerName);
            iSetColor(255, 255, 0);
            iText(50, 550, welcomeText, GLUT_BITMAP_HELVETICA_18);
        }
    }

    else if (currentState == STATE_GAME)
    {
        if (currentLevel == 1)
        {
            iShowImage(0, 0, "assets/images/level1.bmp");
            drawMap(level1);
        }
        else if (currentLevel == 2)
        {
            iShowImage(0, 0, "assets/images/level2.bmp");
            drawMap(level2);
        }
        else if (currentLevel == 3)
        {
            iShowImage(0, 0, "assets/images/level3.bmp");
            drawMap(level3);
        }

        char livesText[20];
        sprintf(livesText, "Lives: %d", lives);
        iText(20, 500, livesText, GLUT_BITMAP_HELVETICA_18);

        iShowSpeed(940, 550);

        // Replace the iSetColor and iFilledCircle with the rotated ball image
        drawRotatedBall(ballX - cameraX - ballRadius, ballY - cameraY - ballRadius, "assets/images/ball.bmp", ballRotation);

        char scoreText[20];
        sprintf(scoreText, "Score: %d", score);
        iSetColor(255, 255, 255);
        iText(20, 560, scoreText, GLUT_BITMAP_HELVETICA_18);

        // Show player name during game
        if (nameEntered && strlen(playerName) > 0)
        {
            char playerText[60];
            sprintf(playerText, "Player: %s", playerName);
            iText(20, 530, playerText, GLUT_BITMAP_HELVETICA_12);
        }
    }
    else if (currentState == STATE_INSTRUCTIONS)
    {
        iShowImage(0, 0, "assets/images/instruction-main.bmp");
        iSetColor(0, 23, 66);
        iText(100, 30, "- Press 'b' to return to the Main Menu.", GLUT_BITMAP_HELVETICA_18);
    }
    else if (currentState == STATE_SETTINGS)
    {
        iShowImage(0, 0, "assets/images/settings_menu.bmp");

        // Sound settings title
        iSetColor(0, 26, 66);
        iText(400, 450, "Sound Settings", GLUT_BITMAP_TIMES_ROMAN_24);

        // Sound ON button - improved visual feedback
        if (soundEnabled)
        {
            iSetColor(0, 200, 0); // Bright green when active
            iFilledRectangle(soundOnBtnX, soundOnBtnY, soundBtnW, soundBtnH);
            iSetColor(255, 255, 255);                                   // White text for contrast
            iRectangle(soundOnBtnX, soundOnBtnY, soundBtnW, soundBtnH); // Border
        }
        else
        {
            iSetColor(60, 60, 60); // Dark gray when inactive
            iFilledRectangle(soundOnBtnX, soundOnBtnY, soundBtnW, soundBtnH);
            iSetColor(150, 150, 150); // Light gray text
        }
        iText(soundOnBtnX + 45, soundOnBtnY + 20, "SOUND ON", GLUT_BITMAP_HELVETICA_18);

        // Sound OFF button - improved visual feedback
        if (!soundEnabled)
        {
            iSetColor(200, 0, 0); // Bright red when active
            iFilledRectangle(soundOffBtnX, soundOffBtnY, soundBtnW, soundBtnH);
            iSetColor(255, 255, 255);                                     // White text for contrast
            iRectangle(soundOffBtnX, soundOffBtnY, soundBtnW, soundBtnH); // Border
        }
        else
        {
            iSetColor(60, 60, 60); // Dark gray when inactive
            iFilledRectangle(soundOffBtnX, soundOffBtnY, soundBtnW, soundBtnH);
            iSetColor(150, 150, 150); // Light gray text
        }
        iText(soundOffBtnX + 40, soundOffBtnY + 20, "SOUND OFF", GLUT_BITMAP_HELVETICA_18);

        // Instructions
        iSetColor(0, 26, 66);
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
        if (currentLevel == 1)
        {
            iShowImage(0, 0, "assets/images/level1.bmp");
            drawMap(level1);
        }
        else if (currentLevel == 2)
        {
            iShowImage(0, 0, "assets/images/level2.bmp");
            drawMap(level2);
        }
        else if (currentLevel == 3)
        {
            iShowImage(0, 0, "assets/images/level3.bmp");
            drawMap(level3);
        }

        // Replace the ball drawing here too
        drawRotatedBall(ballX - cameraX - ballRadius, ballY - cameraY - ballRadius, "assets/images/ball.bmp", ballRotation);
        iShowImage(300, 170, "assets/images/pause.bmp");
    }

    else if (currentState == STATE_ABOUT_US)
    {

        iShowImage(0, 0, "assets/images/about_us_main.bmp");
        iSetColor(0, 23, 66);
        iText(100, 30, "Press 'b' to return to Main Menu", GLUT_BITMAP_HELVETICA_18);
    }

    else if (currentState == STATE_GAME_OVER)
    {
        iShowImage(0, 0, "assets/images/game-over.bmp");
        displayHighScore();

        // Show current player's score
        if (nameEntered && strlen(playerName) > 0)
        {
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
        if (nameEntered && strlen(playerName) > 0)
        {
            char playerScoreText[100];
            sprintf(playerScoreText, "%s completed Level %d! Score: %d", playerName, currentLevel, score);
            iSetColor(255, 255, 0);
            iText(20, 480, playerScoreText, GLUT_BITMAP_HELVETICA_18);
        }
    }
}

void iKeyboard(unsigned char key, int state)
{
    if (state != GLUT_DOWN)
        return;

    // Handle name input
    if (currentState == STATE_PLAYER_NAME_INPUT)
    {
        if (key == '\r' || key == '\n')
        { // Enter key
            if (nameLength > 0)
            {
                nameEntered = true;
                isEnteringName = false;
                currentState = STATE_LEVEL_SELECT;
            }
        }
        else if (key == '\b')
        { // Backspace
            if (nameLength > 0)
            {
                nameLength--;
                playerName[nameLength] = '\0';
            }
        }
        else if (key >= 32 && key <= 126 && nameLength < 49)
        { // Printable characters
            playerName[nameLength] = key;
            nameLength++;
            playerName[nameLength] = '\0';
        }
        return;
    }
    if ((key == 'b' || key == 'B') && currentState == STATE_LEADERBOARD )
    {
        currentState = STATE_MAIN_MENU;
    }
    if ((key == 'b' || key == 'B') && currentState == STATE_LEVEL_SELECT)
    {
        currentState = STATE_MAIN_MENU;
        initializeLevel();
    }

    if ((key == 'b'|| key == 'B' )&& currentState == STATE_PAUSE )
    {
        currentState = STATE_MAIN_MENU;
        initializeLevel();
    }
    if ((key == 'b'|| key == 'B' )&& currentState == STATE_INSTRUCTIONS )
    {
        currentState = STATE_MAIN_MENU;
        initializeLevel();
    }
    if ((key == 'b'|| key == 'B' )&& currentState == STATE_SETTINGS )
    {
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

    if(currentState == STATE_VICTORY)
    {
lives=3;

    }

    if (currentState == STATE_GAME_OVER && (key == 'b' || key == 'B'))
    {
        currentState = STATE_MAIN_MENU;

        initializeLevel();
    }

    if ((key == 'p' || key == 'P')&& currentState == STATE_GAME )
    {
        currentState = STATE_PAUSE;
    }

    if (currentState == STATE_PAUSE && (key == 'r' || key == 'R'))
    {
        currentState = STATE_GAME;
    }

    if (currentState == STATE_PAUSE)
    {
        if (key == 'r' || key == 'R')
        {
            currentState = STATE_GAME;
        }
        else if (key == 'b' || key == 'B')
        {
            currentState = STATE_MAIN_MENU;

            initializeLevel();
        }
    }

    if (currentState == STATE_VICTORY &&( key == 'b' || key == 'B'))
    {
        currentState = STATE_LEVEL_SELECT;
        initializeLevel();
    }
    if (currentState == STATE_ABOUT_US && (key == 'b' || key == 'B'))
    {
        currentState = STATE_MAIN_MENU;
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

    if (currentState == STATE_VICTORY && (key == 'n' || key == 'N'))
{
    // Save score when level is completed
    if (nameEntered && strlen(playerName) > 0)
    {
        savePlayerScore(playerName, score, currentLevel);
    }

    if (currentLevel < 3)
    {
        currentLevel++;
        lives = 3; 
        initializeLevel();
        currentState = STATE_GAME;
    }
    else
    {
        currentState = STATE_MAIN_MENU;
        initializeLevel();
    }
}
}

void iMouseMove(int mx, int my)
{
    if (currentState == STATE_MAIN_MENU)
    {
        if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 5 * (btnH + gap) && my <= btnY + 5 * (btnH + gap) + btnH)
        {
            hoveredButton = 0; // Start button
        }
        else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 4 * (btnH + gap) && my <= btnY + 4 * (btnH + gap) + btnH)
        {
            hoveredButton = 1; // Instructions button
        }
        else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 2 * (btnH + gap) && my <= btnY + 2 * (btnH + gap) + btnH)
        {
            hoveredButton = 2; // Settings button
        }
        else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 3 * (btnH + gap) && my <= btnY + 3 * (btnH + gap) + btnH)
        {
            hoveredButton = 3; // Leaderboard button
        }

        else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 1 * (btnH + gap) && my <= btnY + 1 * (btnH + gap) + btnH)
        {
            hoveredButton = 5; // about us button
        }
        else if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH)
        {
            hoveredButton = 4; // Exit button
        }
        else
        {
            hoveredButton = -1; // No button hovered
        }

        score = 0;
        lives = 3;
    }

    else if (currentState == STATE_LEVEL_SELECT)
    {
        if (mx >= 50 && mx <= 320 && my >= 50 && my <= 300)
        {
            hoveredButton = 0; // Level 1 button
        }
        else if (mx >= 350 && mx <= 620 && my >= 50 && my <= 300)
        {
            hoveredButton = 1; // Level 2 button
        }
        else if (mx >= 650 && mx <= 920 && my >= 50 && my <= 300)
        {
            hoveredButton = 2; // Level 3 button
        }
    }

    else
    {
        hoveredButton = -1;
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
            if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 5 * (btnH + gap) && my <= btnY + 5 * (btnH + gap) + btnH)
            {
                if (!nameEntered || strlen(playerName) == 0)
                {
                    currentState = STATE_PLAYER_NAME_INPUT;
                    isEnteringName = true;
                }
                else
                {
                    currentState = STATE_LEVEL_SELECT;
                }
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 4 * (btnH + gap) && my <= btnY + 4 * (btnH + gap) + btnH)
            {
                currentState = STATE_INSTRUCTIONS;
            }

            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 1 * (btnH + gap) && my <= btnY + 1 * (btnH + gap) + btnH)
            {
                currentState = STATE_ABOUT_US;
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 2 * (btnH + gap) && my <= btnY + 2 * (btnH + gap) + btnH)
            {
                currentState = STATE_SETTINGS;
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 3 * (btnH + gap) && my <= btnY + 3 * (btnH + gap) + btnH)
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
            if (mx >= 310 && mx <= 600 && my >= 290 && my <= 320)
            {
                isEnteringName = true;
            }
            else
            {
                isEnteringName = false;
            }
        }
        else if (currentState == STATE_LEVEL_SELECT)
        {

            if (mx >= 50 && mx <= 320 && my >= 50 && my <= 300)
            {
                currentLevel = 1;
                initializeLevel();
                currentState = STATE_GAME;
            }
            else if (mx >= 350 && mx <= 620 && my >= 50 && my <= 300)
            {
                currentLevel = 2;
                initializeLevel();
                currentState = STATE_GAME;
            }
            else if (mx >= 650 && mx <= 920 && my >= 50 && my <= 300)
            {
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
                if (!soundEnabled)
                { // Only toggle if currently off
                    toggleSound(true);
                }
            }
            // Sound OFF button clicked
            else if (mx >= soundOffBtnX && mx <= soundOffBtnX + soundBtnW &&
                     my >= soundOffBtnY && my <= soundOffBtnY + soundBtnH)
            {
                if (soundEnabled)
                { // Only toggle if currently on
                    toggleSound(false);
                }
            }
        }
    }
}

void iMovement(int value)
{
    updatePhysics();
    glutTimerFunc(8, iMovement, 0);
}

void iSpecialKeyboard(unsigned char key, int state) {}

void iSpecialKeyboardUp(unsigned char key, int state)
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
    glutTimerFunc(8, iMovement, 0);
    iOpenWindow(1000, 600, "Bounce Classic");
    atexit(iFreeSound);

    return 0;
}

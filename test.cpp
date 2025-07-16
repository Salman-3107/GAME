#include "iGraphics.h"
#include "iSound.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

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
bool bgmplaying = false;
bool gameRunning = true;

//co-ordinate
int x,y;
// Ball
float ballX = 200, ballY = 300;
float ballRadius = 20;
float ballDY = 0;  
float ballDx = 0;
float gravity = -0.7;  
bool onGround = false;  

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

//compare
int compareScores(const void *a, const void *b) {
    HighScore *scoreA = (HighScore *)a;
    HighScore *scoreB = (HighScore *)b;
    return scoreB->score - scoreA->score;  // Descending order
}
// Player name system
char playerName[50] = ""; 
int nameLength = 0;
bool nameEntered = false;  // Flag to track if name has been entered this session
bool isEnteringName = false;  // Flag for name input state

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
    "##........................##......##..............................##.................##.........##...##..............#",
    "##.....................P..##......##..............................##.................##.........##...................#",
    "##.............#########..##..##.........#####....................##........c........##.....................#........#",
    "##.............##.....##..##..##.........#####....................##.......##........##.........c....##...###........#",
    "##.............##.............##.........#####............................####..................##...##.....#........#",
    "##xxxx..o....|.##..........c..##.........#####...xx....xx....xx....P.....######......P....xx....##...##.....#...xx..G#",
    "######################################################################################################################"
};

char level3[MAX_MAP_HEIGHT][MAX_MAP_WIDTH] = {
    "################################################################",
    "###...######........##..#####.......##..#....##..##.........P###",
    "###...######..#####..###......##...###.............#...P.....###",
    "###..#####..###########...###..###....c...##..#....##.....######",
    "###....###..#####....#.....P.....##....##....c..........###..###",
    "####....###..####....##...........#...........|..............###",
    "##########....###..##.###..|...P..........###..####.P....#####.#",
    "################################################################"
};

// Camera position
float cameraX = 0, cameraY = 0;
int blockSize = 75;
int visibleWidth = 1000;
int visibleHeight = 600;

// Button variables for Main Menu
int btnX = 800, btnY = 100, btnW = 200, btnH = 50, gap = 20;

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
            strcpy(scores[count].name, tempName);
            scores[count].level = tempLevel;
            scores[count].score = tempScore;
            count++;
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
    iSetColor(0, 255, 0);
    iText(350, 50, "Press 'b' to go back to Main Menu", GLUT_BITMAP_HELVETICA_18);
}

void drawMap(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    // Only draw tiles that are visible on screen
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
            else if(currentChar == 'x'){
                iShowImage(posX, posY, "assets/images/enemy.bmp");
            }
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
            currentState = STATE_GAME_OVER; 
        }
    }

    if (tileYAbove >= 0 && tileYAbove < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (map[tileYAbove][tileX] == '|') {
            currentState = STATE_GAME_OVER; 
        }
    }

    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXLeft >= 0 && tileXLeft < MAX_MAP_WIDTH) {
        if (map[tileYBelow][tileXLeft] == '|') {
            currentState = STATE_GAME_OVER; 
        }
    }

    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXRight >= 0 && tileXRight < MAX_MAP_WIDTH) {
        if (map[tileYBelow][tileXRight] == '|') {
            currentState = STATE_GAME_OVER; 
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
            iPlaySound("assets/sounds/chime.wav", false); 
            map[tileY][tileX] = '.'; 
        }
    }
}

void updatePhysics() {
    if(currentState != STATE_GAME) return;
    
    // Get current level map
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
    
    limitBallPosition();
    updateCameraPosition();
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
            
            // Parse the line format: "Name - Level X: Score"
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

    if (!bgmplaying)
    {
        iPlaySound("assets/sounds/game_audio.wav", true);
        bgmplaying = true;
    }

   if (currentState == STATE_MAIN_MENU)
{
    iShowImage(0, 0, "assets/images/wallpaper.bmp");
   
    iShowImage(btnX, btnY + 4 * (btnH + gap), "assets/images/start.bmp");
    iShowImage(btnX, btnY + 3 * (btnH + gap), "assets/images/instructions.bmp");
    iShowImage(btnX, btnY + (btnH + gap), "assets/images/settings.bmp");
    iShowImage(btnX, btnY + 2 * (btnH + gap), "assets/images/leaderboad.bmp");
    iShowImage(btnX, btnY, "assets/images/Exit.bmp");
}
    else if (currentState == STATE_LEADERBOARD)
{
    iShowImage(0, 0, "assets/images/wallpaper.bmp"); 
    displayLeaderboard();
}
    else if (currentState == STATE_PLAYER_NAME_INPUT) {
        iShowImage(0, 0, "assets/images/name.bmp");
        
        // Draw input box
        iSetColor(255, 255, 255);
        iFilledRectangle(300, 280, 400, 40);
        iSetColor(0, 0, 0);
        iRectangle(300, 280, 400, 40);
        
        // Draw instructions
        iSetColor(0, 0, 0);
        iText(300, 350, "Enter your name:", GLUT_BITMAP_HELVETICA_18);
        iText(300, 250, "Press ENTER when done", GLUT_BITMAP_HELVETICA_12);
        
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
        iText(320, 50, "Select a Level by pressing the number", GLUT_BITMAP_HELVETICA_18);
        
        // Show player name
        if (nameEntered && strlen(playerName) > 0) {
            char welcomeText[80];
            sprintf(welcomeText, "Welcome, %s!", playerName);
            iSetColor(255, 255, 0);
            iText(50, 550, welcomeText, GLUT_BITMAP_HELVETICA_18);
        }
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
        iFilledRectangle(120, 300, 430, 120);
        iSetColor(0, 0, 0);
        iText(120, 400, "Instructions:", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(120, 370, "- Use LEFT and RIGHT arrow keys to move the ball.", GLUT_BITMAP_HELVETICA_18);
        iText(120, 340, "- Prevent the ball from falling below.", GLUT_BITMAP_HELVETICA_18);
        iText(120, 310, "- Press 'b' to return to the Main Menu.", GLUT_BITMAP_HELVETICA_18);
    }
    else if(currentState == STATE_SETTINGS)
    {
        iShowImage(0, 0, "assets/images/settings-main.bmp");
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

    // Rest of keyboard handling
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
    if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 4 * (btnH + gap) && my <= btnY + 4 * (btnH + gap) + btnH)
            {
                 iShowImage(btnX, btnY + 4 * (btnH + gap), "assets/images/start1.bmp");
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 3 * (btnH + gap) && my <= btnY + 3 * (btnH + gap) + btnH)
            {
             iShowImage(btnX, btnY + 3 * (btnH + gap), "assets/images/instructions1.bmp");
   
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + (btnH + gap) && my <= btnY + (btnH + gap) + btnH)
            {
              
              iShowImage(btnX, btnY + (btnH + gap), "assets/images/settings1.bmp");
  
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 2 * (btnH + gap) && my <= btnY + 2 * (btnH + gap) + btnH)
            {
                
              
            iShowImage(btnX, btnY + 2 * (btnH + gap), "assets/images/leaderboad1.bmp");
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH)
            {
           iShowImage(btnX, btnY, "assets/images/exit1.bmp");
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
            if (mx >= btnX && mx <= btnX + btnW && my >= btnY + 4 * (btnH + gap) && my <= btnY + 4 * (btnH + gap) + btnH)
            {
                // Check if player name is already entered
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
                
                 currentState = STATE_LEADERBOARD;  // Add leaderboard button
            }
            else if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH)
            {
                // Reset player name when exiting
                initializePlayerName();
                exit(0);
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
    
    glutInit(&argc, argv);
    iInitializeSound();
    glutTimerFunc(16, iMovement, 0);
    iOpenWindow(1000, 600, "Bounce Classic");
    
    return 0;
}

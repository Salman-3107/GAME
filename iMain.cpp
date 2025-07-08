#include "iGraphics.h"
#include "iSound.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>   

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
} GameState;

GameState currentState = STATE_MAIN_MENU;
bool bgmplaying = false;
//co-ordinate
int x,y;
// Ball
float ballX = 200, ballY = 300;
float ballRadius = 20;
float ballDY = 0;  
float ballDx = 0;
float gravity = -0.6;  
bool onGround = false;  

float vx = 0, vy = 0;
float moveSpeed = 5;
float jumpSpeed = 25; 

int score = 0; 
int currentLevel = 1;
//name 
char playerName[50]; 
int namelength=0;
int write_mode=1;
int entered=0;
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
    "##XXXX..o....|.##..........c..##.........#####...xx....xx....xx....P.....######......P....xx....##...##.....#...xx..G#",
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
int btnX = 100, btnY = 100, btnW = 200, btnH = 50, gap = 20;


void drawMap(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    for (y = 0; y < MAX_MAP_HEIGHT; y++) {
        for (x = 0; x < MAX_MAP_WIDTH; x++) {
            char currentChar = map[y][x];
            int posX = (x * blockSize) - cameraX;  // Apply cameraX for horizontal camera movement
            int posY = ((MAX_MAP_HEIGHT - y - 1) * blockSize) - cameraY;  // Apply cameraY for vertical camera movement

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

    if (ballX - cameraX < leftBound)
        cameraX = ballX - leftBound;
    else if (ballX - cameraX > rightBound)
        cameraX = ballX - rightBound;

    
    cameraX = fmax(0, fmin(cameraX, MAX_MAP_WIDTH * blockSize - visibleWidth));
    cameraY = fmax(0, fmin(cameraY, MAX_MAP_HEIGHT * blockSize - visibleHeight));
}

// Ensure the ball stays within the screen bounds and doesn't go out
void limitBallPosition() {
    if (ballX - ballRadius < cameraX) ballX = cameraX + ballRadius + 300;
    if (ballX + ballRadius > cameraX + visibleWidth) ballX = cameraX + visibleWidth - ballRadius - 300;
    if (ballY - ballRadius < cameraY) ballY = cameraY + ballRadius;
    if (ballY + ballRadius > cameraY + visibleHeight) ballY = cameraY + visibleHeight - ballRadius;
    if (ballY < ballRadius) {
        ballY = ballRadius;  // Place the ball on top of the ground
        ballDY = 0;          // Stop the ball from falling
        onGround = true;     // Set the ball as on the ground
    }
}
void checkSpikeCollision(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    int tileX = (int)(ballX / blockSize); 
    int tileYBelow = (int)((MAX_MAP_HEIGHT * blockSize - (ballY - ballRadius)) / blockSize); // Check below the ball
    int tileYAbove = (int)((MAX_MAP_HEIGHT * blockSize - (ballY + ballRadius)) / blockSize); // Check above the ball
    int tileXLeft = (int)((ballX - ballRadius) / blockSize); // Check left of the ball
    int tileXRight = (int)((ballX + ballRadius) / blockSize); // Check right of the ball

    // Vertical collision with spikes (below the ball)
    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (map[tileYBelow][tileX] == '|') { // Spike collision below
            currentState = STATE_GAME_OVER; 
        }
    }

    // Vertical collision with spikes (above the ball)
    if (tileYAbove >= 0 && tileYAbove < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (map[tileYAbove][tileX] == '|') { // Spike collision above
            currentState = STATE_GAME_OVER; 
        }
    }

    // Horizontal collision with spikes (left side of the ball)
    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXLeft >= 0 && tileXLeft < MAX_MAP_WIDTH) {
        if (map[tileYBelow][tileXLeft] == '|') { // Spike collision left
            currentState = STATE_GAME_OVER; 
        }
    }

    // Horizontal collision with spikes (right side of the ball)
    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXRight >= 0 && tileXRight < MAX_MAP_WIDTH) {
        if (map[tileYBelow][tileXRight] == '|') { // Spike collision right
            currentState = STATE_GAME_OVER; 
        }
    }
}
void checkVictory(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    int tileX = (int)(ballX / blockSize);
    int tileY = (int)((MAX_MAP_HEIGHT * blockSize - ballY) / blockSize);

    // Check if the ball reaches the goal 'G' (green circle)
    if (tileY >= 0 && tileY < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (map[tileY][tileX] == 'G') { // Victory condition reached
            currentState = STATE_VICTORY; // Trigger victory state
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
            if (map[tileYCenter][tileXLeft] == '#')  // Collision with left side
            {
                ballX = (tileXLeft + 1) * blockSize + ballRadius;
            }
        }

        if (tileYCenter >= 0 && tileYCenter < MAX_MAP_HEIGHT && tileXRight >= 0 && tileXRight < MAX_MAP_WIDTH)
        {
            if (map[tileYCenter][tileXRight] == '#')  // Collision with right side
            {
                ballX = tileXRight * blockSize - ballRadius;
            }
        }
}
void collection(char map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH]) {
    int tileX = (int)(ballX / blockSize); 
    int tileY = (int)((MAX_MAP_HEIGHT * blockSize - (ballY)) / blockSize);

    // Check coin collection
    if (tileY >= 0 && tileY < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (map[tileY][tileX] == 'c') { 
            score += 10;  
            iPlaySound("assets/sounds/chime.wav", false); 
            map[tileY][tileX] = '.'; 
        }
    }
}

void saveHighScore(int score) {
    FILE *file = fopen("highscore.txt", "a+");
    if (file != NULL) {
        fprintf(file, "Player: %d\n", score);  
        fclose(file);
    }
}
void gameOver() {
    saveHighScore(score); 
    currentState = STATE_GAME_OVER;
}
void displayHighScore() {
    FILE *file = fopen("highscore.txt", "r");
    if (file != NULL) {
        char line[100];
        int highScore = 0;
        while (fgets(line, sizeof(line), file)) {
            int currentScore = 0;
            sscanf(line, "Player: %d", &currentScore);
            if (currentScore > highScore) {
                highScore = currentScore;
            }
        }
        fclose(file);

        char highScoreText[20];
        sprintf(highScoreText, "High Score: %d", highScore);
        iSetColor(255, 255, 255);
        iText(20, 520, highScoreText, GLUT_BITMAP_HELVETICA_18);
    }
}
void iKeyboard(unsigned char key,int state)
{
    if (key == 'b' && currentState == STATE_PAUSE) 
    {
        currentState = STATE_MAIN_MENU;
        ballX = 200;
        ballY = 300;
        ballDY = 0;
        vx = 0;
    }

    if (key == ' ' && onGround)  
    {
        ballDY = jumpSpeed;
        onGround = false;
    }

    if (currentState == STATE_GAME_OVER && key == 'b')  
    {
        currentState = STATE_MAIN_MENU;  
        ballX = 200;  
        ballY = 300;
        ballDY = 0;  
        vx = 0;
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
        else if (key == 'b' && currentState == STATE_PAUSE)  
        {
            currentState = STATE_MAIN_MENU;
        }
    }
    if (currentState == STATE_LEVEL_SELECT) 
    {
        if (key == '1') 
        {
            currentState = STATE_GAME;
            currentLevel = 1;
        }
        else if (key == '2') 
        {
            currentState = STATE_GAME;
            currentLevel = 2;
        }
        else if (key == '3')  
        {
            currentState = STATE_GAME;
            currentLevel = 3;
        }
    }

    if (currentState == STATE_VICTORY && key == 'n') 
    {
        if (currentLevel < 3) {
            currentLevel++;
            x=0;
            y=0;
            currentState = STATE_GAME;
        } else {
            currentState = STATE_MAIN_MENU;
        }
    }
     if (currentState == STATE_PLAYER_NAME_INPUT && key != ' ' && key != 13) {  
        if ((key >= 65 && key <= 90) || (key >= 97 && key <= 122) || key == 8) { 
            if (key == 8) {  // Backspace
                if (namelength > 0)
                    playerName[--namelength] = '\0';
            } else {
                if (namelength < 49) {
                    playerName[namelength++] = key;
                    playerName[namelength] = '\0';
                }
            }
        }
    }

    if (key == 13 && currentState!=STATE_GAME) {  // Enter key
        if (strlen(playerName) > 0) {  
            currentState = STATE_LEVEL_SELECT; 
        }
    }
}


void iDraw()
{
    iClear();
    //iShowImage(0, 0, "wallpaper/wallpaper.bmp");

    if (!bgmplaying)
    {
        iPlaySound("assets/sounds/game_audio.wav", true);
        bgmplaying = true;
    }

    if (currentState == STATE_MAIN_MENU)
    {
         iClear();
         iShowImage(0, 0, "assets/images/wallpaper.bmp");
        iSetColor(0, 255, 0);
        iFilledRectangle(btnX, btnY, btnW, btnH);
        iSetColor(0, 0, 0);
        iText(btnX + 70, btnY + 15, "Start", GLUT_BITMAP_HELVETICA_18);

        iSetColor(0, 200, 255);
        iFilledRectangle(btnX + 1 * (btnW + gap), btnY, btnW, btnH);
        iSetColor(0, 0, 0);
        iText(btnX + 1 * (btnW + gap) + 40, btnY + 15, "Instructions", GLUT_BITMAP_HELVETICA_18);

        iSetColor(255, 165, 0);
        iFilledRectangle(btnX + 2 * (btnW + gap), btnY, btnW, btnH);
        iSetColor(0, 0, 0);
        iText(btnX + 2 * (btnW + gap) + 60, btnY + 15, "Settings", GLUT_BITMAP_HELVETICA_18);

        iSetColor(255, 0, 0);
        iFilledRectangle(btnX + 3 * (btnW + gap), btnY, btnW, btnH);
        iSetColor(0, 0, 0);
        iText(btnX + 3 * (btnW + gap) + 70, btnY + 15, "Exit", GLUT_BITMAP_HELVETICA_18);
    }
    else if (currentState == STATE_PLAYER_NAME_INPUT) {
        iClear();
        iShowImage(0, 0, "assets/images/name.bmp");
        iText(300, 300, playerName, GLUT_BITMAP_HELVETICA_18);
    }
    else if (currentState == STATE_LEVEL_SELECT)
    {
        iClear();
         iShowImage(0, 0, "assets/images/level_select.bmp");
         iSetColor(255, 255, 255);  
         iText(320, 50, "Select a Level by pressing the number", GLUT_BITMAP_HELVETICA_18);

    }

    else if (currentState == STATE_GAME)
    {
        iClear();
      
       if (currentLevel == 1) {
         iShowImage(0, 0, "assets/images/level1.bmp");
            drawMap(level1);
            collision(level1);
             checkSpikeCollision(level1);
             checkVictory(level1);
              collection(level1);
        } else if (currentLevel == 2) {
             iShowImage(0, 0, "assets/images/level2.bmp");
           drawMap(level2);
            collision(level2);
             checkSpikeCollision(level2);
             checkVictory(level2);
                collection(level2);
        } else if (currentLevel == 3) {
             iShowImage(0, 0, "assets/images/level3.bmp");
           drawMap(level3);
            collision(level3);
             checkSpikeCollision(level3);
             checkVictory(level3);
                collection(level3);
        }

        ballDY += gravity;
        ballY += ballDY;
        ballX += ballDx;

        

        if (ballY < ballRadius)
        {
            ballY = ballRadius;
            ballDY = 0;
            onGround = true;
        }

        limitBallPosition();
        updateCameraPosition();
       

        iSetColor(255, 0, 0);
        iFilledCircle(ballX - cameraX, ballY - cameraY, ballRadius);

       char scoreText[20];
       sprintf(scoreText, "Score: %d", score);
       iSetColor(255, 255, 255);
    iText(20, 560, scoreText, GLUT_BITMAP_HELVETICA_18);
    }

    else if (currentState == STATE_INSTRUCTIONS)
    {
         iClear();
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
        iClear();
        iShowImage(0, 0, "assets/images/settings-main.bmp");
    }

   else if (currentState == STATE_PAUSE)
    {
        iClear();
    

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

    // Draw the ball at its last position
    iSetColor(255, 0, 0);
    iFilledCircle(ballX - cameraX, ballY - cameraY, ballRadius);

    
     iShowImage(300, 170, "assets/images/pause.bmp");
    
    }
    else if (currentState == STATE_GAME_OVER)
    {
        iShowImage(0, 0, "assets/images/game-over.bmp");
        saveHighScore(score);
        displayHighScore();
    }

    else if (currentState == STATE_VICTORY)
    {
        iShowImage(0, 0, "assets/images/victory.bmp");
        saveHighScore(score);
        displayHighScore();
    }
}

void iMouseMove(int mx, int my) {}
void iMouseDrag(int mx, int my) {}
void iMouseWheel(int dir, int mx, int my) {}

void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (currentState == STATE_MAIN_MENU)
        {
            if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH)
            {
                 
                currentState =  STATE_PLAYER_NAME_INPUT;
            }
            else if (mx >= btnX + 1 * (btnW + gap) && mx <= btnX + 1 * (btnW + gap) + btnW &&
                     my >= btnY && my <= btnY + btnH)
            {
                currentState = STATE_INSTRUCTIONS;
            }
            else if (mx >= btnX + 2 * (btnW + gap) && mx <= btnX + 2 * (btnW + gap) + btnW &&
                     my >= btnY && my <= btnY + btnH)
            {
                currentState = STATE_SETTINGS;
            }
            else if (mx >= btnX + 3 * (btnW + gap) && mx <= btnX + 3 * (btnW + gap) + btnW &&
                     my >= btnY && my <= btnY + btnH)
            {
                exit(0);
            }
        }
    }
}

void iMovement(int value)
{
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
        ballDx = 0;  // Stop horizontal movement if no key is pressed
    }
     if(isSpecialKeyPressed(GLUT_KEY_UP))
    {
        if (onGround)  
        {
            ballDY = jumpSpeed;  
            onGround = false;  
        }
    }
    glutTimerFunc(10, iMovement, 0); // Call this function again after 15 milliseconds
   
}
void iSpecialKeyboard(unsigned char key, int state){}

// void iSpecialKeyboard(unsigned char key, int state)
// {
//     int moveSpeed = 10;


//     if (key==GLUT_KEY_LEFT && state == GLUT_DOWN)
//     {
//         ballDx=-moveSpeed;
//     }
//     else if(key==GLUT_KEY_LEFT && state == GLUT_UP)
//     {
//         ballDx = 0;
//     }
//     if (key==GLUT_KEY_RIGHT && state == GLUT_DOWN)
//     {
//         ballDx=+moveSpeed;
//     }
//     else if(key==GLUT_KEY_RIGHT && state == GLUT_UP)
//     {
//         ballDx = 0;
//     }
// }
void iSpecialKeyboardUp(unsigned char key,int state)
{
    if (key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT)
    {
        ballDx = 0;  
    }
}


//int gameFPS = 60; 

void timer(int value) {
   
    if (currentState != STATE_PAUSE)
    {
        iDraw();
    }

    glutTimerFunc(15, timer, 0); 

}

int main(int argc, char *argv[])
{
    printf("Starting game...\n");

    glutInit(&argc, argv);
    iInitializeSound();
    glutTimerFunc(10, iMovement, 0);
    glutTimerFunc(15, timer, 0);  
    iOpenWindow(1000, 600, "Bounce Classic");
    // int t=iSetTimer(15,iMovement);
    return 0;
}

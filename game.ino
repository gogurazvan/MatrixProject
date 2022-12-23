#include <LiquidCrystal.h>
#include <string.h>
#include "LedControl.h"
#include <EEPROM.h>

#define MIN_TRESHHOLD 200
#define MAX_TRESHHOLD 800

#define UP 0
#define DOWN 1
#define LEFT 3
#define RIGHT 4

#define MAIN 0
#define MAIN_OPTION_NR 5
#define PLAY 1
#define HIGHSCORE 2
#define SETTINGS 3
#define HOW_TO_PLAY 4
#define ABOUT 5

#define SET_OPTION_NR 6
#define DIFFICULTY 0
#define CONTRAST 1
#define BRIGHTNESS 2
#define M_BRIGHTNESS 3
#define SOUND 4

#define ARROW_LEFT 2
#define ARROW_RIGHT 1
#define ARROW_SELECT 0
#define BLOCK 3

const byte contrastPin = 10;
const byte brightPin = 3;
const byte rs = 9;
const byte en = 8;
const byte d4 = 13;
const byte d5 = 12;
const byte d6 = 7;
const byte d7 = 4;

const byte dinPin = 5;
const byte clockPin = 1;
const byte loadPin = 6;
const byte matrixSize = 8;

const byte buzzerPin = 11;

const byte pinButton = 2;
const byte xPin = A0;
const byte yPin = A1;

byte change = true;//detects if lcd refresh is needed
//settings
byte sound = true;
byte difficulty = 2;
byte brightness = 3;
byte contrast = 2;
byte mBrightness = 3;
byte highscores[5] = {0, 0, 0, 0, 0};

int eeAddress = 0;
struct eePromable{
  byte sound, difficulty, brightness, mBrightness, contrast;
  int topScores[5];
};

const unsigned long debounceInterval = 200; //interrupt debounce
unsigned long lastPressDebounce = 0;
byte buttonPressed = false;

String mainMenu[MAIN_OPTION_NR] = {
  "Start Game",
  "Highscore",
  "Settings",
  "How To Play",
  "About"
};

String settingsMenu[SET_OPTION_NR] = {
  "Difficulty",
  "Contrast",
  "Brightness",
  "Matrix Brightness",
  "Sound",
  "Back to Menu"
};

String backMsg = "Press to go back";
char aboutMsg[] = "Snake by Gogu Razvan     github: https://github.com/gogurazvan/         ";
char howMsg[] = "Use the joystick to control the snake and eat the food for big score     ";

String noScoreMsg = "No scores"; 

byte option = 0; //determines the option selected and where the option arrow should be
byte collumn = 0;

byte menuState = MAIN;
byte settingSelected = false;
//custom characters for menu
byte selectArrow[] = {
  B11000,
  B11100,
  B11110,
  B11111,
  B11111,
  B11110,
  B11100,
  B11000
};
byte rightArrow[] = {
  B00000,
  B10000,
  B11000,
  B11100,
  B11110,
  B11100,
  B11000,
  B10000
};
byte leftArrow[] = {
  B00000,
  B00001,
  B00011,
  B00111,
  B01111,
  B00111,
  B00011,
  B00001
};
byte block[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte setupPlay = true; //determines if the game is just beginning
byte optiunePauza = 0;
byte pauza = 0;

int score = 0;
int direction = RIGHT; //determines in wich direction the snake goes when it moves
struct Punct{
  int x, y;  
}newFoodPos, lastFoodPos; 
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;

const byte moveInterval =500; //determines snake movement speed
unsigned long long lastMoved = 0;

byte matrix[matrixSize][matrixSize] = { //reprezentation of led matrix
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
byte matrixByte[matrixSize] = {
  B00000000,
  B01000100,
  B00101000,
  B00010000,
  B00010000,
  B00010000,
  B00000000,
  B00000000
};
byte matrixChanged = true; //checks if the matrix needs to be refreshed

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

void setup() {
  pinMode(pinButton, INPUT_PULLUP);
  pinMode(contrastPin, OUTPUT);
  pinMode(brightPin, OUTPUT);
  
  analogWrite(contrastPin,50);
  analogWrite(brightPin,128);

  lcd.begin(16, 2);
  lcd.createChar(ARROW_SELECT, selectArrow);
  lcd.createChar(ARROW_RIGHT, rightArrow);
  lcd.createChar(ARROW_LEFT, leftArrow);
  lcd.createChar(BLOCK, block);

  lc.shutdown(0, false);
  lc.clearDisplay(0);  

  matrix[xPos][yPos] =1;
  
  attachInterrupt(digitalPinToInterrupt(pinButton), buttonPress, RISING);
}

void loop() {
  setupLCD();
  switch(menuState) {
    case MAIN:
      collumn = moveOptions(MAIN_OPTION_NR);
      if (buttonPressed){
        change = true; 
        menuState = option + 1;
        option = 0;
        collumn = 0;
        buttonPressed = false;
        break;
      }
      if (change){
        change = false;      
        afisareMain();
      }
      break;

    case SETTINGS:
      if (!settingSelected){
        collumn = moveOptions(SET_OPTION_NR);
        if (buttonPressed){
          change = true;           
          if (option == SET_OPTION_NR-1){
            menuState = MAIN;
            option = 0;
            collumn = 0;
              for (int row = 0; row < matrixSize; row++) {
                for (int col = 0; col < matrixSize; col++) {
                  lc.setLed(0, row, col, false); // turns on LED at col, row
                }
              }         
          }else{
            settingSelected = true;
          }
          buttonPressed = false;
          break;
        }
      }else{
        setSetting();
        if (buttonPressed){
          settingSelected = false;
          buttonPressed = false;
          change = true;
          break;
        }
      } 
      if (change){
        change = false;
        afisareSettings();
      }
      break;

    case HOW_TO_PLAY:
      afisareHow();
      if (buttonPressed){
        menuState = MAIN;
        option = 0;
        collumn = 0;
        buttonPressed = false;
      }    
      break;

    case ABOUT:
      afisareAbout();
      if (buttonPressed){
        menuState = MAIN;
        option = 0;
        collumn = 0;
        buttonPressed = false;
      }    
      break;
    case HIGHSCORE:
      // afisareScor();
      if (buttonPressed){
        menuState = MAIN;
        option = 0;
        collumn = 0;
        buttonPressed = false;
      }  
      break;
    case PLAY:
      if (setupPlay){
        setupPlay = false;
        generateFood();        
      }
      if (pauza == 0){
        if (buttonPressed){
          pauza = 1;
          buttonPressed = false;
          change = true;
          break;
        }
        afisareScor();
        modifyDirection();
        if (millis() - lastMoved > moveInterval){ 
          updatePositions();
          lastMoved = millis();
        }
        if (matrixChanged){
          updateMatrix();
          matrixChanged = false;           
        }   
        // gameOver();
      }else{
        if (buttonPressed){
          buttonPressed = false;
          change = true;
          if (optiunePauza) menuState = MAIN;
          pauza = 0; 
          break;
        }
        afiseazaPauza();
        optiunePauza = alegeriPauza(optiunePauza);
        
      }
      break;
    default:
      menuState = MAIN;
  }

}
void afiseazaPauza(){ //displays the options of the pause screen
  if (change){
    lcd.clear();
    change = false;
  }
  
  lcd.setCursor(0,0);
  lcd.print("Paused");

  lcd.setCursor(1,1);
  lcd.print("Continue");

  lcd.setCursor(12,1);
  lcd.print("Exit");

  lcd.setCursor(11*optiunePauza,1);
  lcd.write(byte(ARROW_SELECT));
}

byte alegeriPauza(byte opt){ //selects option of the pause screen
  int xVal = analogRead(xPin);
  if (xVal < MIN_TRESHHOLD){ 
    if (opt!=0) change = true;  
    return 0;
  }
  if (xVal > MAX_TRESHHOLD){
    if (opt!=1) change = true; 
    return 1;
  }
  return opt;
}

void setupLCD(){//apply settings
  int mappedBrightness = map(brightness,0,6,0,255);
  int mappedContrast = map(contrast,0,6,0,100);
  int mappedMBrightness = map(mBrightness, 0, 6, 0, 15);
  analogWrite(contrastPin,mappedContrast);
  analogWrite(brightPin,mappedBrightness);
  lc.setIntensity(0, mappedMBrightness);
}

void afisareMain(){ //display main menu on lcd
  lcd.clear();
  lcd.setCursor(0, collumn); 
  lcd.write(byte(ARROW_SELECT));
  lcd.print(mainMenu[option]);

  if (collumn == 0){
    lcd.setCursor(1, 1);
    if (option + 1 == MAIN_OPTION_NR)
      lcd.print(mainMenu[0]);
    else
      lcd.print(mainMenu[option+1]);
  }else{
    lcd.setCursor(1, 0);
    if (option == 0)
      lcd.print(mainMenu[MAIN_OPTION_NR-1]);
    else
      lcd.print(mainMenu[option-1]);
  } 
}

void afisareSettings(){ //display seetings menu on lcd
  lcd.clear();
  lcd.setCursor(0, collumn);
  if (settingSelected){
    lcd.write(byte(ARROW_LEFT));
    if (option == DIFFICULTY){
      for(int i = 1; i <= difficulty; ++i) lcd.write(byte(BLOCK));
      lcd.setCursor(4, collumn);
    }
    if (option == CONTRAST){
      for(int i = 1; i <= contrast; ++i) lcd.write(byte(BLOCK));
      lcd.setCursor(7, collumn);
    }
    if (option == BRIGHTNESS){
      for(int i = 1; i <= brightness; ++i) lcd.write(byte(BLOCK));
      lcd.setCursor(7, collumn);
    }
    if (option == M_BRIGHTNESS){
      for(int i = 1; i <= mBrightness; ++i) lcd.write(byte(BLOCK));
      lcd.setCursor(7, collumn);
    }
    if (option == SOUND){
      if (sound) lcd.print("On");
      else lcd.print("Off");
    }
    lcd.write(byte(ARROW_RIGHT));
  }else{ 
    lcd.write(byte(ARROW_SELECT));
    lcd.print(settingsMenu[option]);
  }

  if (collumn == 0){
    lcd.setCursor(1, 1);
    if (option + 1 == SET_OPTION_NR)
      lcd.print(settingsMenu[0]);
    else
      lcd.print(settingsMenu[option+1]);
  }else{
    lcd.setCursor(1, 0);
    if (option == 0)
      lcd.print(settingsMenu[SET_OPTION_NR-1]);
    else
      lcd.print(settingsMenu[option-1]);
  }
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, true); // turns on LED at col, row
    }
  }
}

void afisareAbout(){ //display about text on lcd with scrolling text
  static unsigned long lastShift = 0;
  const unsigned long interval = 500;
  static int positionStart = 0;
  if (millis()-lastShift > interval){
    lcd.clear();
    delay(2);
    for (int i = 0; i < 16; ++i) {
      lcd.print(aboutMsg[(positionStart+i)%strlen(aboutMsg)]);
    }
    positionStart = (positionStart+1)%strlen(aboutMsg);
    
    lcd.setCursor(0, 1);
    lcd.print(backMsg);
    lastShift = millis();
  }
}
void afisareHow(){ //display how to play text on lcd with scrolling text
  static unsigned long lastShift = 0;
  const unsigned long interval = 500;
  static int positionStart = 0;
  if (millis()-lastShift > interval){
    lcd.clear();
    delay(2);
    for (int i = 0; i < 16; ++i) {
      lcd.print(howMsg[(positionStart+i)%strlen(howMsg)]);
    }
    positionStart = (positionStart+1)%strlen(howMsg);
    
    lcd.setCursor(0, 1);
    lcd.print(backMsg);
    lastShift = millis();
  }
  return;  
}

void afisareScor(){ //display in-game hood
  if (change){
    lcd.clear();
    change = false;
  }
  
  lcd.setCursor(0,0);
  lcd.print("Level:");

  lcd.setCursor(0,1);
  lcd.print(difficulty);

  lcd.setCursor(9,0);
  lcd.print("Score:");

  lcd.setCursor(9,1);
  lcd.print(score);
  
}

byte moveOptions(byte maxOption) { //handles the menu select
  static byte yNeutral = false;
  int yVal = analogRead(yPin);
  if (yVal > MAX_TRESHHOLD && !yNeutral){
    option += 1;
    if (option == maxOption) option = 0;
    yNeutral = true;  
    change = true;  
    return 1;
  }
    
  if (yVal < MIN_TRESHHOLD && !yNeutral){
    if (option == 0) option = maxOption;
    option -= 1;
    yNeutral = true;  
    change = true;  
    return 0;
  }
  if (yVal < MAX_TRESHHOLD && yVal > MIN_TRESHHOLD)
    yNeutral = false;
  return collumn;
}

void setSetting(){ //handles the change of the selected settings option
  static byte xNeutral = false;
  int xVal = analogRead(xPin);
  int incr = 0;
  if (xVal > MAX_TRESHHOLD && !xNeutral){
    if (sound)
        tone(buzzerPin,2500,20);
    incr = 1;
    xNeutral = true;   
    change = true; 
  }
    
  if (xVal < MIN_TRESHHOLD && !xNeutral){
    if (sound)
        tone(buzzerPin,2500,20);
    incr = -1;
    xNeutral = true;   
    change = true;  
  }

  if (xVal > MIN_TRESHHOLD && xVal < MAX_TRESHHOLD)
    xNeutral = false;

  if (option == DIFFICULTY){
    difficulty += incr;
    if (difficulty<1) difficulty = 1;
    if (difficulty>3) difficulty = 3;
    return;
  }
  if (option == CONTRAST){
    contrast += incr;
    if (contrast<1) contrast = 1;
    if (contrast>6) contrast = 6;
    return;
  }
  if (option == BRIGHTNESS){
    brightness += incr;
    if (brightness<1) brightness = 1;
    if (brightness>6) brightness = 6;
    return;
  }
  if (option == M_BRIGHTNESS){
    mBrightness += incr;
    if (mBrightness<1) mBrightness = 1;
    if (mBrightness>6) mBrightness = 6;
    return;
  }
  if (option == SOUND){
    if (incr<0) sound = false;
    if (incr>0) sound = true;
    return;
  }
}
void generateFood() { //generates points for the snake to eat
  lastFoodPos.x = xPos;
  lastFoodPos.y = yPos;
  newFoodPos.x = random(matrixSize);
  newFoodPos.y = random(matrixSize);
  matrix[lastFoodPos.x][lastFoodPos.x] = 0;
  matrix[newFoodPos.x][newFoodPos.x] = 1;
  matrixChanged = true;
}

void updateByteMatrix() {
  for(int row =0; row < matrixSize; row++) {
    lc.setRow(0, row, matrixByte[row]);
  }
}
void updateMatrix() {
  for(int row =0; row < matrixSize; row++) {
    for(int col =0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col]);
    }
  }
}

void modifyDirection(){ //change direction where snake moves
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);
  if (xValue < MIN_TRESHHOLD) direction = LEFT;
  if (xValue > MAX_TRESHHOLD) direction = RIGHT;
  if (yValue < MIN_TRESHHOLD) direction = DOWN;
  if (yValue > MAX_TRESHHOLD) direction = UP;
}

void updatePositions() { //moves the snake

  xLastPos = xPos;
  yLastPos = yPos;

  if (direction == LEFT) {
    if (xPos < matrixSize - 1) {
      xPos++;
    }else {
      xPos = 0;
    }
  }

  if (direction == RIGHT) {
    if (xPos > 0) {
      xPos--;
    }else {
      xPos = matrixSize - 1;
    }
  }

  if (direction == UP) {
    if (yPos < matrixSize - 1) {
      yPos++;
    }else {
      yPos = 0;
    }
  }

  if (direction == DOWN) {
    if (yPos > 0) {
      yPos--;
    }else {
      yPos = matrixSize - 1;
    }
  }

  matrixChanged = true;
  if (matrix[xPos][yPos]==1){
    score++;
    generateFood();  
    if (sound)
      tone(buzzerPin, 1150, 35);
  }
  matrix[xLastPos][yLastPos] = 0;
  matrix[xPos][yPos] = 1;
  
}
void buttonPress(){ //interrupt function detects button pressing
  if (millis() - lastPressDebounce > debounceInterval) {
      buttonPressed = true;
      lastPressDebounce = millis();
      if (sound)
        tone(buzzerPin,1500,30);
  }
}

void gameOver(){
  return;
}

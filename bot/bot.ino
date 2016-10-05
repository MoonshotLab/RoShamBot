#include <Servo.h>

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

int rainbowInc = 0;

int currentMode = 0;
int *currentModePtr  = &currentMode;

int playerScore = 0;
int botScore = 0;

int readyCount = 0; // fill ring to start game

int pos = 0;    // variable to store the servo position
int upperFingersPin = 8;
int lowerFingersPin = 7;

#define NEOPIN 6
int fullNeoLength = 50;
int neoLength = 42;
int halfNeoLength = neoLength / 2;
int userPixelOffset = 2;
int botPixelOffset = halfNeoLength + userPixelOffset + 2; // accounts for two display lights in the center

int upperOpen = 85;
int upperClosed = 0;
int upperRest = (upperOpen + upperClosed) / 2;

int lowerOpen = 0;
int lowerClosed = 85;
int lowerRest = (lowerOpen + lowerClosed) / 2;

int throwDelay = 1000; // how many ms delay is shown

// hand servos
Servo upperFingers, lowerFingers;
// Servo thumb;

// score display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// ring strips
Adafruit_NeoPixel strip = Adafruit_NeoPixel(fullNeoLength, NEOPIN, NEO_GRB + NEO_KHZ800); // GRB, not RGB!

void playNeutral() {
  upperFingers.write(upperRest);
  lowerFingers.write(lowerRest);
  Serial.println("neutral");
}

void playRock() {
  upperFingers.write(upperClosed);
  lowerFingers.write(lowerClosed);
  Serial.println("playing rock");
}

void playPaper() {
  upperFingers.write(upperOpen);
  lowerFingers.write(lowerOpen);
  Serial.println("playing paper");
}

void playScissors() {
  upperFingers.write(upperOpen);
  lowerFingers.write(lowerClosed);
  Serial.println("playing scissors");
}

void playMove(int input) {
  switch(input) {
    case 0:
      playRock();
      break;
    case 1:
      playPaper();
      break;
    case 2:
      playScissors();
      break;
    case 3:
      playNeutral();
      break;
     default:
       exit(0);
  }
}

void neoWipe() {
  for (int i = 0; i < fullNeoLength; i++) {
    strip.setPixelColor(i, 0);
  }

  strip.show();
}

void playerWipe() {
  for (int i = 0; i < halfNeoLength; i++) {
    strip.setPixelColor(i, 0);
  }

  strip.show();
}

void botWipe() {
  for (int i = 0; i < halfNeoLength; i++) {
    strip.setPixelColor(halfNeoLength + i, 0);
  }

  strip.show();
}

void lightPlayerThird(int segment, int color, bool wipe) {
  if (wipe) neoWipe();

  segment = segment % 3;

  for (int i = 0; i < halfNeoLength / 3; i++) {
    int pos = (halfNeoLength / 3 * segment) + i;
    strip.setPixelColor(pos, color);
  }

  strip.show();
}

void lightPlayerRing(int color, bool wipe) {
  if (wipe) neoWipe();

  for (int i = 0; i < halfNeoLength; i++) {
    strip.setPixelColor(i, color);
  }

  strip.show();
}

void lightBotRing(int color, bool wipe) {
  if (wipe) neoWipe();

  for (int i = 0; i < halfNeoLength; i++) {
    strip.setPixelColor(halfNeoLength + i, color);
  }

  strip.show();
}

void readPlayerRock(bool wipe) {
  lightPlayerThird(0, strip.Color(0, 0, 255), wipe); // blue
}

void readPlayerPaper(bool wipe) {
  lightPlayerThird(1, strip.Color(0, 0, 255), wipe); // blue
}
void readPlayerScissors(bool wipe) {
  lightPlayerThird(2, strip.Color(0, 0, 255), wipe); // blue
}

void readPlayerError(bool wipe) {
  lightPlayerRing(strip.Color(255, 0, 0), wipe); // red
}

void countdownOne() {
  neoWipe();

  for (int i  = 0; i < halfNeoLength / 3; i++) {
    int posUser = i + userPixelOffset;
    int posBot = i + botPixelOffset;

    strip.setPixelColor(posUser, strip.Color(255, 255, 255));
    strip.setPixelColor(posBot, strip.Color(255, 255, 255));
    strip.show();
    delay(10);
  }
}

void countdownTwo() {
  // fill in first segment
  for (int i  = 0; i < halfNeoLength / 3; i++) {
    int posUser = i + userPixelOffset;
    int posBot = i + botPixelOffset;

    strip.setPixelColor(posUser, strip.Color(255, 255, 255));
    strip.setPixelColor(posBot, strip.Color(255, 255, 255));
  }
  strip.show();

  // animate fill in second
  for (int i  = 0; i < halfNeoLength / 3; i++) {
    int posUser = (halfNeoLength / 3) + i + userPixelOffset;
    int posBot = (halfNeoLength / 3) + i + botPixelOffset;

    strip.setPixelColor(posUser, strip.Color(255, 255, 255));
    strip.setPixelColor(posBot, strip.Color(255, 255, 255));
    strip.show();
    delay(10);
  }
}

void countdownThree() {
  // fill in first two segment
  for (int i  = 0; i < 2 * halfNeoLength / 3; i++) {
    int posUser = i + userPixelOffset;
    int posBot = i + botPixelOffset;

    strip.setPixelColor(posUser, strip.Color(255, 255, 255));
    strip.setPixelColor(posBot, strip.Color(255, 255, 255));
  }
  strip.show();

  // animate fill in third
  for (int i  = 0; i < halfNeoLength / 3; i++) {
    int posUser = (2 * halfNeoLength / 3) + i + userPixelOffset;
    int posBot = (2 * halfNeoLength / 3) + i + botPixelOffset;

    strip.setPixelColor(posUser, strip.Color(255, 255, 255));
    strip.setPixelColor(posBot, strip.Color(255, 255, 255));
    strip.show();
    delay(10);
  }
}

void countdownThrow() {
  int flashRepeat = 3;
  int flashColors[] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255)};

  for (int j = 0; j < flashRepeat; j++) {
    for (int i = 0; i < halfNeoLength; i++) {
      int posUser = i + userPixelOffset;
      int posBot = i + botPixelOffset;

      strip.setPixelColor(posUser, flashColors[j]);
      strip.setPixelColor(posBot, flashColors[j]);
    }
    strip.show();
    delay(100);
  }

  for (int i = 0; i < halfNeoLength; i++) {
    int posUser = i + userPixelOffset;
    int posBot = i + botPixelOffset;

    strip.setPixelColor(posUser, strip.Color(255, 255, 255));
    strip.setPixelColor(posBot, strip.Color(255, 255, 255));
  }
  strip.show();
  delay(throwDelay);
  neoWipe();
}


void lightNeoRing(int input) {
  switch(input) {
    case 4:
      // read scissors
      readPlayerScissors(true);
      break;
    case 5:
      // read paper
      readPlayerPaper(true);
      break;
    case 6:
      // read rock
      readPlayerRock(true);
      break;
    case 7:
      // read error
      readPlayerError(true);
      break;
    case 8:
      // count 1
      countdownOne();
      break;
    case 9:
      // count 2
      countdownTwo();
      break;
    case 10:
      // count 3
      countdownThree();
      break;
    case 11:
      // count throw
      countdownThrow();
      break;
  }
}

void displayScore(int playerScore, int botScore) {
  // convert to chars
  char playerScoreChar = playerScore + '0';
  char botScoreChar = botScore + '0';

  alpha4.writeDigitAscii(0, playerScoreChar); // convert to char
  alpha4.writeDigitAscii(1, ' ');
  alpha4.writeDigitAscii(2, ' ');
  alpha4.writeDigitAscii(3, botScoreChar); // covert to char
  alpha4.writeDisplay();
}

void playerOneWin() {
  neoWipe();

  for (int i = 0; i < halfNeoLength; i++) {
    uint32_t green = strip.Color(255, 0, 0);
    uint32_t red = strip.Color(0, 255, 0);

    strip.setPixelColor(i, red);
    strip.setPixelColor(i + 24, green);
  }

  strip.show();
  delay(2000);
}

void playerTwoWin() {
  neoWipe();

  for (int i = 0; i < halfNeoLength; i++) {
    uint32_t green = strip.Color(255, 0, 0);
    uint32_t red = strip.Color(0, 255, 0);

    strip.setPixelColor(i, green);
    strip.setPixelColor(i + halfNeoLength, red);
  }

  strip.show();
  delay(2000);
}

void neoCountdown() {
  neoWipe();

  for (int i = 0; i < halfNeoLength; i++) {
    uint32_t blue = strip.Color(0, 0, 255);
    strip.setPixelColor(i, blue);
    strip.setPixelColor(i + halfNeoLength, blue);
    if (i != 0 && i % (halfNeoLength / 3) == 0) {
      strip.show();
      delay(1000);
    }
  }

  strip.show();
  delay(1000);
}

void rainbowCycleInc(int val, int wait) {
  val = val % 255;

  for(int i = 0; i < halfNeoLength; i++) {
    strip.setPixelColor(i + userPixelOffset, Wheel(((i * 256 / strip.numPixels()) + val) & 255));
  }

  strip.show();
  delay(wait);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void displayFillRing() {
  neoWipe();

  int steps = 20;
  float fillPct = halfNeoLength / steps * readyCount;

  if (readyCount > 0) {
    for (int i = 0; i < fillPct; i++) {
      strip.setPixelColor(i + userPixelOffset, strip.Color(255, 255, 255));
    }
    strip.show();
  }
}

void botHandTest() {
  neoWipe();
}

void playerWinsOverall() {
  neoWipe();
}

void botWinsOverall() {
  neoWipe();
}

void setup() {
  // exit(0);
  Serial.begin(9600);

  alpha4.begin(0x70);  // pass in the address
  alpha4.clear();
  alpha4.writeDisplay();

  displayScore(playerScore, botScore);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  upperFingers.attach(upperFingersPin);
  lowerFingers.attach(lowerFingersPin);

  upperFingers.write(upperRest);
  lowerFingers.write(lowerRest);
}

void loop() {
  while(Serial.available()==0){
    if (currentMode == 0) {
      rainbowCycleInc(rainbowInc++, 20);

      if (rainbowInc > 255) {
        rainbowInc = 0;
      }

    }
  }

  int input = Serial.read();

  Serial.println(input);

  if (input >= 0 && input < 4) {
    playMove(input);
  } else if (input >= 4 && input < 12) {
    lightNeoRing(input);
  } else if (input >= 12 && input < 15) {
    switch(input) {
      case 12:
        playerScore++;
        break;
      case 13:
        botScore++;
        break;
      case 14:
        // reset both scores
        playerScore = 0;
        botScore = 0;
        break;
    }

    displayScore(playerScore, botScore);
  } else if (input == 15) {
    // clear display
    currentMode = -1;
    neoWipe();
  } else if (input == 16) {
    currentMode = 0; // rainbow cycle
  } else if (input >= 17 && input < 19) {
    currentMode = 1;
    switch(input) {
      case 17:
        // fill ring inc
        readyCount++;
        break;
      case 18:
        // fill ring dec
        readyCount--;

        if (readyCount == 0) {
          currentMode = 0; // rainbow cycle
        }
        break;
    }

    displayFillRing();
  } else if (input == 19) {
    currentMode = 2;
    botHandTest();
  } else if (input == 20) {
    playerWinsOverall();
  } else if (input == 21) {
    botWinsOverall();
  }
//  delay(1000);
}

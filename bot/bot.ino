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

int throwDelay = 2000; // how many ms throw is shown

// hand servos
Servo upperFingers, lowerFingers;
// Servo thumb;

// score display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// ring strips
Adafruit_NeoPixel strip = Adafruit_NeoPixel(fullNeoLength, NEOPIN, NEO_GRB + NEO_KHZ800); // GRB, not RGB!

// wtf grb
uint32_t red = strip.Color(0, 255, 0);
uint32_t green = strip.Color(255, 0, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t orange = strip.Color(165, 255, 0);
uint32_t white = strip.Color(255, 255, 255);
uint32_t purple = strip.Color(0, 128, 128);

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
  delay(100);
  Serial.write("moveDone");
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

void lightPlayerThird(int segment, uint32_t color, bool wipe) {
  if (wipe) neoWipe();

  segment = segment % 3;

  for (int i = 0; i < halfNeoLength / 3; i++) {
    int pos = (halfNeoLength / 3 * segment) + i;
    strip.setPixelColor(pos, color);
  }

  strip.show();
}

void lightPlayerRing(uint32_t color, bool wipe) {
  if (wipe) {
    neoWipe();
  }

  for (int i = 0; i < halfNeoLength; i++) {
    strip.setPixelColor(i + userPixelOffset, color);
  }

  strip.show();
}

void lightBotRing(uint32_t color, bool wipe) {
  if (wipe) {
    neoWipe();
  }

  for (int i = 0; i < halfNeoLength; i++) {
    strip.setPixelColor(i + botPixelOffset, color);
  }

  strip.show();
}

void readPlayerRock(int res, bool wipe) {
  uint32_t color;
  if (res == -1) {
    // lose, red
    color = red;
  } else if (res == 0) {
    // tie, purple
    color = purple;
  } else if (res == 1) {
    // win, green
    color = green;
  }

  lightPlayerThird(0, color, wipe);
}

void readPlayerPaper(int res, bool wipe) {
  uint32_t color;
  if (res == -1) {
    // lose, red
    color = red;
  } else if (res == 0) {
    // tie, purple
    color = purple;
  } else if (res == 1) {
    // win, green
    color = green;
  }

  lightPlayerThird(1, color, wipe);
}
void readPlayerScissors(int res, bool wipe) {
  uint32_t color;
  if (res == -1) {
    // lose, red
    color = red;
  } else if (res == 0) {
    // tie, purple
    color = purple;
  } else if (res == 1) {
    // win, green
    color = green;
  }

  lightPlayerThird(2, color, wipe);
}

void readPlayerError(bool wipe) {
  lightPlayerRing(red, wipe);
  delay(500);
  Serial.write("errorDone");
}

void countdownOne(bool user) {
  neoWipe();

  for (int i  = 0; i < halfNeoLength / 3; i++) {
    if (user) {
      int posUser = i + userPixelOffset;
      strip.setPixelColor(posUser, white);
    }

    int posBot = i + botPixelOffset;
    strip.setPixelColor(posBot, white);
    strip.show();
    delay(10);
  }

  Serial.write("oneDone");
}

void countdownTwo(bool user) {
  // fill in first segment
  for (int i  = 0; i < halfNeoLength / 3; i++) {
    if (user) {
      int posUser = i + userPixelOffset;
      strip.setPixelColor(posUser, white);
    }

    int posBot = i + botPixelOffset;
    strip.setPixelColor(posBot, white);
  }
  strip.show();

  // animate fill in second
  for (int i  = 0; i < halfNeoLength / 3; i++) {
    if (user) {
      int posUser = (halfNeoLength / 3) + i + userPixelOffset;
      strip.setPixelColor(posUser, white);
    }

    int posBot = (halfNeoLength / 3) + i + botPixelOffset;
    strip.setPixelColor(posBot, white);
    strip.show();
    delay(10);
  }

  Serial.write("twoDone");
}

void countdownThree(bool user) {
  // fill in first two segment
  for (int i  = 0; i < 2 * halfNeoLength / 3; i++) {
    if (user) {
      int posUser = i + userPixelOffset;
      strip.setPixelColor(posUser, white);
    }

    int posBot = i + botPixelOffset;
    strip.setPixelColor(posBot, white);
  }
  strip.show();

  // animate fill in third
  for (int i  = 0; i < halfNeoLength / 3; i++) {
    if (user) {
      int posUser = (2 * halfNeoLength / 3) + i + userPixelOffset;
      strip.setPixelColor(posUser, white);
    }

    int posBot = (2 * halfNeoLength / 3) + i + botPixelOffset;
    strip.setPixelColor(posBot, white);
    strip.show();
    delay(10);
  }

  Serial.write("threeDone");
}

void countdownThrow(bool user) {
  int flashRepeat = 3;
  uint32_t flashColors[3] = {red, green, blue};

  for (int j = 0; j < flashRepeat; j++) {
    for (int i = 0; i < halfNeoLength; i++) {
      if (user) {
        int posUser = i + userPixelOffset;
        strip.setPixelColor(posUser, flashColors[j]);
      }

      int posBot = i + botPixelOffset;
      strip.setPixelColor(posBot, flashColors[j]);
    }
    strip.show();
    delay(100);
  }

  for (int i = 0; i < halfNeoLength; i++) {
    if (user) {
      int posUser = i + userPixelOffset;
      strip.setPixelColor(posUser, white);
    }

    int posBot = i + botPixelOffset;
    strip.setPixelColor(posBot, white);
  }
  strip.show();
  delay(throwDelay);
  neoWipe();

  Serial.write("throwDone");
}


void lightNeoRing(int input) {
  switch(input) {
    case 4:
      // read scissors
      // readPlayerScissors(true);
      break;
    case 5:
      // read paper
      // readPlayerPaper(true);
      break;
    case 6:
      // read rock
      // readPlayerRock(true);
      break;
    case 7:
      // read error
      readPlayerError(false);
      break;
    case 8:
      // count 1
      countdownOne(true);
      break;
    case 9:
      // count 2
      countdownTwo(true);
      break;
    case 10:
      // count 3
      countdownThree(true);
      break;
    case 11:
      // count throw
      countdownThrow(true);
      break;
  }
}

void wipeDisplay() {
  alpha4.writeDigitAscii(0, ' ');
  alpha4.writeDigitAscii(1, ' ');
  alpha4.writeDigitAscii(2, ' ');
  alpha4.writeDigitAscii(3, ' ');
  alpha4.writeDisplay();
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

void displayChars(char c0, char c1, char c2, char c3) {
  alpha4.writeDigitAscii(0, c0);
  alpha4.writeDigitAscii(1, c1);
  alpha4.writeDigitAscii(2, c2);
  alpha4.writeDigitAscii(3, c3);

  alpha4.writeDisplay();
}

// void playerOneWin() {
//   neoWipe();
//
//   for (int i = 0; i < halfNeoLength; i++) {
//     uint32_t green = strip.Color(255, 0, 0);
//     uint32_t red = green;
//
//     strip.setPixelColor(i, red);
//     strip.setPixelColor(i + 24, green);
//   }
//
//   strip.show();
//   delay(2000);
// }
//
// void playerTwoWin() {
//   neoWipe();
//
//   for (int i = 0; i < halfNeoLength; i++) {
//     uint32_t green = strip.Color(255, 0, 0);
//     uint32_t red = green;
//
//     strip.setPixelColor(i, green);
//     strip.setPixelColor(i + halfNeoLength, red);
//   }
//
//   strip.show();
//   delay(2000);
// }

void neoCountdown() {
  neoWipe();

  for (int i = 0; i < halfNeoLength; i++) {
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
      strip.setPixelColor(i + userPixelOffset, white);
    }
    strip.show();
  }
}

void botHandIntro() {
  neoWipe();

  int flashRepeat = 3;
  uint32_t flashColors[3] = {red, green, blue};
  for (int j = 0; j < flashRepeat; j++) {
    for (int i = 0; i < halfNeoLength; i++) {
      int posUser = i + userPixelOffset;
      strip.setPixelColor(posUser, flashColors[j]);
      int posBot = i + botPixelOffset;
      strip.setPixelColor(posBot, flashColors[j]);
    }
    strip.show();
    delay(100);
  }

  neoWipe();

  int timingDelay = 500;
  delay(timingDelay * 3); // dramatic pause

  countdownOne(false);
  delay(timingDelay);
  countdownTwo(false);
  delay(timingDelay);
  countdownThree(false);
  delay(timingDelay);
  countdownThrow(false);

  playRock();
  delay(timingDelay);
  playScissors();
  delay(timingDelay);
  playPaper();
  delay(timingDelay);

  playNeutral();

  displayChars('P', 'L', 'A', 'Y');
  delay(timingDelay * 3);

  wipeDisplay();

  int numFlashes = 2;

  // flash user
  for (int i = 0; i < numFlashes; i++) {
    displayChars('0', ' ', ' ', ' ');
    lightPlayerRing(white, true);
    delay(timingDelay);

    neoWipe();
    delay(timingDelay);
  }

  // flash bot
  for (int i = 0; i < numFlashes; i++) {
    displayChars(' ', ' ', ' ', '0');
    lightBotRing(white, true);
    delay(timingDelay);

    neoWipe();
    delay(timingDelay);
  }

  neoWipe();
  displayScore(0, 0);
  delay(timingDelay);
  Serial.write("introDone");
}

void playerVictor(bool playerWon, uint32_t c, uint8_t wait) {
  neoWipe();

  for (int j=0; j<20; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      if (playerWon) {
        for (uint16_t i=0; i < halfNeoLength; i = i + 3) {
          strip.setPixelColor(i + userPixelOffset + q, c); //turn every third pixel on
        }
        strip.show();

        delay(wait);

        for (uint16_t i=0; i < halfNeoLength; i = i + 3) {
          strip.setPixelColor(i + userPixelOffset + q, 0); //turn every third pixel off
        }
      } else {
        // bot won
        for (uint16_t i=0; i < halfNeoLength; i = i + 3) {
          strip.setPixelColor(i + botPixelOffset + q, c); //turn every third pixel on
        }
        strip.show();

        delay(wait);

        for (uint16_t i=0; i < halfNeoLength; i = i + 3) {
          strip.setPixelColor(i + botPixelOffset + q, 0); //turn every third pixel off
        }
      }
    }
  }

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
    delay(250);
    Serial.write("wipeDone");
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
    botHandIntro();
    readyCount = 0;

  } else if (input == 20) {
    // playerWinsOverall(); // remove
  } else if (input == 21) {
    // botWinsOverall(); // remove
  } else if (input == 22) {
    playerVictor(true, green, 20);
    delay(250);
    Serial.write("victoryDone");
  } else if (input == 23) {
    playerVictor(false, green, 20);
    delay(250);
    Serial.write("victoryDone");
  } else if (input >= 24 && input < 27) {
    switch(input) {
      case 24:
        // player win rock
        readPlayerRock(1, false);
        break;
      case 25:
        // player tie rock
        readPlayerRock(0, false);
        break;
      case 26:
        // player lose rock
        readPlayerRock(-1, false);
        break;
    }
  } else if (input >= 27 && input < 30) {
    switch(input) {
      case 27:
        // player win paper
        readPlayerPaper(1, false);
        break;
      case 28:
        // player tie paper
        readPlayerPaper(0, false);
        break;
      case 29:
        // player lose paper
        readPlayerPaper(-1, false);
        break;
    }
  } else if (input >= 30 && input < 33) {
    switch(input) {
      case 30:
        // player win scissors
        readPlayerScissors(1, false);
        break;
      case 31:
        // player tie scissors
        readPlayerScissors(0, false);
        break;
      case 32:
        // player lose scissors
        readPlayerScissors(-1, false);
        break;
    }
  } else if (input >= 33 && input < 36) {
    switch(input) {
      case 33:
        // bot win
        lightBotRing(green, false);
        break;
      case 34:
        // bot tie
        lightBotRing(purple, false);
        break;
      case 35:
        // bot lose
        lightBotRing(red, false);
        break;
    }

    delay(250);
    Serial.write("botResultDone");
  }
//  delay(1000);
}

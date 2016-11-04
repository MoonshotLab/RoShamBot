#include <Adafruit_TiCoServo.h>

#include <String.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LED_RELAY_PIN 3
#define UPPER_FINGERS_PIN 9
#define LOWER_FINGERS_PIN 10
#define NEOPIN 11
#define RESET_BUTTON_PIN 12

int rainbowInc = 0;

int currentMode = 0;

int playerScore = 0;
int botScore = 0;

bool sleeping = false;

int readyCount = 0; // fill ring to start game

int pos = 0;    // variable to store the servo position

int resetButtonState = 0;

const int fullNeoLength = 50;
const int neoLength = 42;
const int halfNeoLength = neoLength / 2;
const int userPixelOffset = 2;
const int botPixelOffset = halfNeoLength + userPixelOffset; // accounts for two display lights in the center

const int upperOpen = 175;
const int upperClosed = 61;
const int upperRest = 121;

const int lowerOpen = 5;
const int lowerClosed = 121;
const int lowerRest = 60;

const int throwDelay = 2000; // how many ms throw is shown

// hand servos
Adafruit_TiCoServo upperFingers, lowerFingers;

// score display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// ring strips
Adafruit_NeoPixel strip = Adafruit_NeoPixel(fullNeoLength, NEOPIN, NEO_GRB + NEO_KHZ800); // GRB, not RGB!

// wtf grb
const uint32_t red = strip.Color(0, 255, 0);
const uint32_t green = strip.Color(255, 0, 0);
const uint32_t blue = strip.Color(0, 0, 255);
const uint32_t orange = strip.Color(165, 255, 0);
const uint32_t white = strip.Color(255, 255, 255);
const uint32_t purple = strip.Color(0, 128, 128);

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

// previously I was writing strings via serial, but I want something of a set size
uint8_t encodeMsg(String str) {
  // I wish I could do a switch here, but the val has to be an int :/
  uint8_t res;

  if (str == "reset") {
    res = 1;
  } else if (str == "resetDone") {
    res = 2;
  } else if (str == "displayCleared") {
    res = 3;
  } else if (str == "oneDone") {
    res = 4;
  } else if (str == "twoDone") {
    res = 5;
  } else if (str == "threeDone") {
    res = 6;
  } else if (str == "throwDone") {
    res = 7;
  } else if (str == "moveDone") {
    res = 8;
  } else if (str == "errorDone") {
    res = 9;
  } else if (str == "wipeDone") {
    res = 10;
  } else if (str == "victoryDone") {
    res = 11;
  } else if (str == "botResultDone") {
    res = 12;
  } else {
    res = 13;
  }

  return res;
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
  // Serial.println("moveDone");
  Serial.println(encodeMsg("moveDone"));
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
    int pos = (halfNeoLength / 3 * segment) + userPixelOffset + i;
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
  Serial.println("errorDone");
  // Serial.println(encodeMsg("errorDone"));
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

  Serial.println("oneDone");
  // Serial.println(encodeMsg("oneDone"));
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

  Serial.println("twoDone");
  // Serial.println(encodeMsg("twoDone"));
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

  Serial.println("threeDone");
  // Serial.println(encodeMsg("threeDone"));
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
  delay(250);
  neoWipe();

  Serial.println("throwDone");
  // Serial.println(encodeMsg("throwDone"));
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

  alpha4.writeDigitAscii(0, botScoreChar); // convert to char
  alpha4.writeDigitAscii(1, ' ');
  alpha4.writeDigitAscii(2, ' ');
  alpha4.writeDigitAscii(3, playerScoreChar); // covert to char
  alpha4.writeDisplay();
}

void displayChars(char c0, char c1, char c2, char c3) {
  alpha4.writeDigitAscii(0, c0);
  alpha4.writeDigitAscii(1, c1);
  alpha4.writeDigitAscii(2, c2);
  alpha4.writeDigitAscii(3, c3);

  alpha4.writeDisplay();
}

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

void rainbowCycleInc(int val, int wait) {
  val = val % 255;

  for(int i = 0; i < halfNeoLength; i++) {
    strip.setPixelColor(i + userPixelOffset, Wheel(((i * 256 / strip.numPixels()) + val) & 255));
  }

  strip.show();
  delay(wait);
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
}

void playerVictor(bool playerWon, uint32_t c, uint8_t wait) {
  neoWipe();

  for (int j=0; j<40; j++) {  //do 40 cycles of chasing
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

void hideDisplay() {
  alpha4.clear();
  alpha4.writeDisplay();
}

void relaysOn() {
  digitalWrite(LED_RELAY_PIN, HIGH);
  // digitalWrite(servoRelayPin, HIGH);
}

void relaysOff() {
  digitalWrite(LED_RELAY_PIN, LOW);
  // digitalWrite(servoRelayPin, LOW);
}


void reset(bool sleep) {
  if (sleep) {
    currentMode = -1;
    sleeping = true;
  } else {
    currentMode = 0;
  }

  playerScore = 0;
  botScore = 0;

  hideDisplay();
  playNeutral();

  if (sleep) {
    relaysOff();
  }
}

void setup() {
  // exit(0);
  Serial.begin(14400);

  pinMode(RESET_BUTTON_PIN, INPUT);
  pinMode(LED_RELAY_PIN, OUTPUT);
  digitalWrite(LED_RELAY_PIN, HIGH);
  // pinMode(servoRelayPin, OUTPUT);

  alpha4.begin(0x70);  // pass in the address
  alpha4.clear();
  alpha4.writeDisplay();

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  upperFingers.attach(UPPER_FINGERS_PIN);
  lowerFingers.attach(LOWER_FINGERS_PIN);

  upperFingers.write(upperRest);
  lowerFingers.write(lowerRest);
}

void loop() {
  while (sleeping) {
    resetButtonState = digitalRead(RESET_BUTTON_PIN);

    if (resetButtonState == HIGH) {
      sleeping = false;
      relaysOn();
      delay(100);
      Serial.println("reset");
      // Serial.println(encodeMsg("reset"));
    }

    delay(250);
  }

  if(Serial.available()==0) {
    resetButtonState = digitalRead(RESET_BUTTON_PIN);

    if (resetButtonState == HIGH) {
      sleeping = false;
      Serial.println("reset");
      // Serial.println(encodeMsg("reset"));
      delay(100);
    }

    if (!sleeping && currentMode == 0) {
      rainbowCycleInc(rainbowInc++, 20);

      if (rainbowInc > 255) {
        rainbowInc = 0;
      }
    }

    delay(100);
  } else {
    int input = Serial.read();

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
      delay(100);
      Serial.println("wipeDone");
      // Serial.println(encodeMsg("wipeDone"));
    } else if (input == 16) {
      currentMode = 0; // rainbow cycle
      playerScore = 0;
      botScore = 0;
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
      // botHandIntroOld();
      botHandIntro();
      readyCount = 0;

    } else if (input == 20) {
      // playerWinsOverall(); // remove
    } else if (input == 21) {
      // botWinsOverall(); // remove
    } else if (input == 22 || input == 23) {
      switch(input) {
        case 22:
          // player wins
          playerVictor(true, green, 20);
          break;
        case 23:
          // bot wins
          playerVictor(false, green, 20);
          break;
      }

      delay(250);
      Serial.println("victoryDone");
      // Serial.println(encodeMsg("victoryDone"));
      reset(false); // reset but don't sleep
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
      Serial.println("botResultDone");
      // Serial.println(encodeMsg("botResultDone"));
    } else if (input == 36) {
      hideDisplay();
      delay(100);
      Serial.println("displayCleared");
      // Serial.println(encodeMsg("displayCleared"));
    } else if (input == 37) {
      reset(true); // reset and sleep
      delay(100);
      Serial.println("resetDone");
      // Serial.println(encodeMsg("resetDone"));
    }
  }
}

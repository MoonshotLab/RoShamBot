#include <Servo.h>

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

int currentMode;

int playerScore = 0;
int botScore = 0;

int readyCount = 0; // fill ring to start game

int pos = 0;    // variable to store the servo position
int upperFingersPin = 7;
int lowerFingersPin = 8;

#define NEOPIN 6
int neoLength = 42;
int halfNeoLength = neoLength / 2;
int userPixelOffset = 2;

int upperOpen = 10;
int upperClosed = 150;
int upperRest = 80;

int lowerOpen = 170;
int lowerClosed = 90;
int lowerRest = 140;

// hand servos
Servo upperFingers, lowerFingers;
// Servo thumb;

// score display
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// ring strips
Adafruit_NeoPixel strip = Adafruit_NeoPixel(neoLength, NEOPIN, NEO_GRB + NEO_KHZ800); // GRB, not RGB!

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
  for (int i = 0; i < neoLength; i++) {
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
  lightPlayerRing(strip.Color(0, 255, 0), wipe); // red
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
      // countdownOne();
      break;
    case 9:
      // count 2
      // countdownTwo();
      break;
    case 10:
      // count 3
      // countdownThree();
      break;
    case 11:
      // count throw
      // countdownThrow();
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

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  while (true) {
    for(j=0; j<256; j++) { // 5 cycles of all colors on wheel
      for(i = 0; i < halfNeoLength; i++) {
        strip.setPixelColor(i + userPixelOffset, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
      strip.show();
      delay(wait);
    }
  }
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
  //
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
  while(Serial.available()==0){};

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
    neoWipe();
  } else if (input == 16) {
    currentMode = 0;
    rainbowCycle(20);
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
          currentMode = 1;
          rainbowCycle(20);
        }
        break;
    }

    displayFillRing();
  } else if (input == 19) {
    // botHandTest();
  }
//  delay(1000);
}

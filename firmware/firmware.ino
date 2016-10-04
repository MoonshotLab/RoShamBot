#include <Servo.h>

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

int playerScore = 0;
int botScore = 0;

int pos = 0;    // variable to store the servo position

int readScissorsPin = 22;
int readPaperPin = 23;
int readRockPin = 24;
int readErrorPin = 25;
int countOnePin = 51;
int countTwoPin = 52;
int countThreePin = 53;
int countThrowPin = 50;

int neoPin = 0;
int neoLength = 50;
int halfNeoLength = neoLength / 2;

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
Adafruit_NeoPixel strip = Adafruit_NeoPixel(neoLength, neoPin, NEO_GRB + NEO_KHZ800); // GRB, not RGB!

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

void lightLed(int input) {
  switch(input) {
    case 4:
      // read scissors
      digitalWrite(readScissorsPin, HIGH);
      digitalWrite(readPaperPin, LOW);
      digitalWrite(readRockPin, LOW);
      digitalWrite(readErrorPin, LOW);
      digitalWrite(countOnePin, LOW);
      digitalWrite(countTwoPin, LOW);
      digitalWrite(countThreePin, LOW);
      digitalWrite(countThrowPin, LOW);
      delay(2500);
      break;
    case 5:
      // read paper
      digitalWrite(readScissorsPin, LOW);
      digitalWrite(readPaperPin, HIGH);
      digitalWrite(readRockPin, LOW);
      digitalWrite(readErrorPin, LOW);
      digitalWrite(countOnePin, LOW);
      digitalWrite(countTwoPin, LOW);
      digitalWrite(countThreePin, LOW);
      digitalWrite(countThrowPin, LOW);
      delay(2500);
      break;
    case 6:
      // read rock
      digitalWrite(readScissorsPin, LOW);
      digitalWrite(readPaperPin, LOW);
      digitalWrite(readRockPin, HIGH);
      digitalWrite(readErrorPin, LOW);
      digitalWrite(countOnePin, LOW);
      digitalWrite(countTwoPin, LOW);
      digitalWrite(countThreePin, LOW);
      digitalWrite(countThrowPin, LOW);
      delay(2500);
      break;
    case 7:
      // read error
      digitalWrite(readScissorsPin, LOW);
      digitalWrite(readPaperPin, LOW);
      digitalWrite(readRockPin, LOW);
      digitalWrite(readErrorPin, HIGH);
      digitalWrite(countOnePin, LOW);
      digitalWrite(countTwoPin, LOW);
      digitalWrite(countThreePin, LOW);
      digitalWrite(countThrowPin, LOW);
      delay(2500);
      break;
    case 8:
      // count 1
      digitalWrite(readScissorsPin, LOW);
      digitalWrite(readPaperPin, LOW);
      digitalWrite(readRockPin, LOW);
      digitalWrite(readErrorPin, LOW);
      digitalWrite(countOnePin, HIGH);
      digitalWrite(countTwoPin, LOW);
      digitalWrite(countThreePin, LOW);
      digitalWrite(countThrowPin, LOW);
      delay(900);
      break;
    case 9:
      // count 2
      digitalWrite(readScissorsPin, LOW);
      digitalWrite(readPaperPin, LOW);
      digitalWrite(readRockPin, LOW);
      digitalWrite(readErrorPin, LOW);
      digitalWrite(countOnePin, LOW);
      digitalWrite(countTwoPin, HIGH);
      digitalWrite(countThreePin, LOW);
      digitalWrite(countThrowPin, LOW);
      delay(900);
      break;
    case 10:
      // count 3
      digitalWrite(readScissorsPin, LOW);
      digitalWrite(readPaperPin, LOW);
      digitalWrite(readRockPin, LOW);
      digitalWrite(readErrorPin, LOW);
      digitalWrite(countOnePin, LOW);
      digitalWrite(countTwoPin, LOW);
      digitalWrite(countThreePin, HIGH);
      digitalWrite(countThrowPin, LOW);
      delay(900);
      break;
    case 11:
      // count throw
      digitalWrite(readScissorsPin, LOW);
      digitalWrite(readPaperPin, LOW);
      digitalWrite(readRockPin, LOW);
      digitalWrite(readErrorPin, LOW);
      digitalWrite(countOnePin, LOW);
      digitalWrite(countTwoPin, LOW);
      digitalWrite(countThreePin, LOW);
      digitalWrite(countThrowPin, HIGH);
      delay(100);
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

void neoWipe() {
  for (int i = 0; i < neoLength; i++) {
    strip.setPixelColor(i, 0);
  }

  strip.show();
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

void setup() {
  // exit(0);
  Serial.begin(9600);

  alpha4.begin(0x70);  // pass in the address
  alpha4.clear();
  alpha4.writeDisplay();

  displayScore(playerScore, botScore);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  upperFingers.attach(3);
  lowerFingers.attach(6);
  // thumb.attach(9);

  upperFingers.write(upperRest);
  lowerFingers.write(lowerRest);
  // thumb.write(150);

  pinMode(readScissorsPin, OUTPUT);
  pinMode(readPaperPin, OUTPUT);
  pinMode(readRockPin, OUTPUT);
  pinMode(readErrorPin, OUTPUT);
  pinMode(countOnePin, OUTPUT);
  pinMode(countTwoPin, OUTPUT);
  pinMode(countThreePin, OUTPUT);
  pinMode(countThrowPin, OUTPUT);
}

void loop() {
  while(Serial.available()==0){
    digitalWrite(readScissorsPin, LOW);
    digitalWrite(readPaperPin, LOW);
    digitalWrite(readRockPin, LOW);
    digitalWrite(readErrorPin, LOW);
    digitalWrite(countOnePin, LOW);
    digitalWrite(countTwoPin, LOW);
    digitalWrite(countThreePin, LOW);
    digitalWrite(countThrowPin, LOW);
  };

  int input = Serial.read();

  Serial.println(input);

  if (input >= 0 && input < 4) {
    playMove(input);
  } else if (input >= 4 && input < 12) {
    lightLed(input);
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
  }
//  delay(1000);
}

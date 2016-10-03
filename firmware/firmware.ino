#include <Servo.h>

Servo upperFingers, lowerFingers;
// Servo thumb;

int input;
int pos = 0;    // variable to store the servo position

int readScissorsPin = 22;
int readPaperPin = 23;
int readRockPin = 24;
int readErrorPin = 25;
int countOnePin = 51;
int countTwoPin = 52;
int countThreePin = 53;
int countThrowPin = 50;

int upperOpen = 10;
int upperClosed = 150;
int upperRest = 80;

int lowerOpen = 170;
int lowerClosed = 90;
int lowerRest = 140;

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

void setup() {
  exit(0);
  Serial.begin(9600);

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
  input = Serial.read();

  Serial.println(input);

  if (input >= 0 && input < 4) {
    playMove(input);
  } else if (input >= 4 && input < 12) {
    // throw leds
    lightLed(input);
  }

//  delay(1000);
}

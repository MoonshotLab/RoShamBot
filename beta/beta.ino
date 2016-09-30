#include <Servo.h>

Servo upperFingers, lowerFingers, thumb;
int input;

int readScissorsPin = 21;
int readPaperPin = 23;
int readRockPin = 24;
int readErrorPin = 25;
int countOnePin = 50;
int countTwoPin = 51;
int countThreePin = 52;
int countThrowPin = 53;


// digital scissors 21
// digital paper 23
// digital rock 24
// digital error 25

// 1     50
// 2     51
// 3     52
// THROW 53


// upper fingers open 170
// upper fingers neutral 140
// upper fingers closed 110

// lower fingers open 120
// lower fingers neutral 145
// lower fingers closed 170

// thumb open 150
// thumb neutral 135
// thumb closed 120


int pos = 0;    // variable to store the servo position

void playNeutral() {
  upperFingers.write(160);
  lowerFingers.write(160);
  thumb.write(150);
  Serial.println("neutral");
}

void playRock() {
  upperFingers.write(110);
  lowerFingers.write(170);
  thumb.write(120);
  Serial.println("playing rock");
}

void playPaper() {
  upperFingers.write(170);
  lowerFingers.write(120);
  thumb.write(150);
  Serial.println("playing paper");
}

void playScissors() {
  upperFingers.write(170);
  lowerFingers.write(170);
  thumb.write(120);
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
      delay(900);
      break;
  }
}

void setup() {
//  exit(0);
  Serial.begin(9600);

  upperFingers.attach(3);
  lowerFingers.attach(6);
  thumb.attach(9);

  upperFingers.write(170);
  lowerFingers.write(120);
  thumb.write(150);

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
  while(Serial.available()==0){};
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

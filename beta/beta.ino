#include <Servo.h>

Servo upperFingers, lowerFingers, thumb;
int move;

// const int OPEN = 180;
// const int CLOSE = 0;

// upper fingers open 170
// upper fingers closed 120

// lower fingers open 120
// lower fingers closed 170

// thumb open 150
// thumb closed 120


int pos = 0;    // variable to store the servo position

void playRock() {
  upperFingers.write(120);
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

void playMove(int move) {
  switch(move) {
    case 0:
      playRock();
      break;
    case 1:
      playPaper();
      break;
    case 2:
      playScissors();
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
  upperFingers.write(120);
  thumb.write(150);
}

void loop() {
  while(Serial.available()==0){};
  move = Serial.read();

  if (move >= 0 && move < 3) {
    playMove(move);
  }

  delay(1000);
}

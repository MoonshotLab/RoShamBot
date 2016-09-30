#include <Servo.h>

Servo upperFingers, lowerFingers, thumb;
int move;

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
  upperFingers.write(140);
  lowerFingers.write(145);
  thumb.write(135);
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
    case 3:
      playNeutral();
      break;
     default:
       exit(0);
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
}

void loop() {
  while(Serial.available()==0){};
  move = Serial.read();

  if (move >= 0 && move < 3) {
    playMove(move);
  }

  delay(1000);
}

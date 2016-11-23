## RoShamBot

Artificially intelligent rock paper scissors player. Read more [here](https://medium.com/moonshotlab/man-vs-machine-learning-40a39f7f936#.fnjjtdat2).

![Alt text](/assets/roshambot-2.jpeg?raw=true "Roshambot")

### Requirements
* Python (2.7)
    * [pip](https://pip.pypa.io/en/stable/installing/)
    * [virtualenv](https://virtualenv.pypa.io/en/stable/installation/)
    * Package requirements listed in `requirements.txt`. (`pip install -r requirements.txt`)
* Leap Motion
    * [Desktop SDK](https://developer.leapmotion.com/v2)
    * Other libraries, as per https://api.leapmotion.com/documentation/python/devguide/Project_Setup.html
* [Platformio](http://docs.platformio.org/en/stable/installation.html)
* Arduino Libraries
  * Adafruit TiCoServo
  * Adafruit LED Backpack
  * Adafruit GFX

### Do tha thang
* Upload the sketch in `bot/bot.ino` to your Arduino.
* `source run`

### Weirdness
* Servos and NeoPixels don't play nicely together. Using Adafruit's TiCoServo library solved lots of my problems. For more info, read [this](https://learn.adafruit.com/neopixels-and-servos/the-ticoservo-library).
* Keep `src/main.cpp` and `bot/bot.ino` in sync. Platformio uses the prior and the Arduino IDE uses the latter. At some point Platformio stopped uploading the sketch so it's currently just being used to get port info and other config settings to pass to the Python script. In theory, you shouldn't have to manually upload to the Arduino.  ¯\_(ツ)_/¯

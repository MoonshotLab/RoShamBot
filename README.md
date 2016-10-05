## ROSHAMBOT 3000

### Requirements
* Python (2.7)
    * Package requirements listed in `requirements.txt`. (`pip install -r requirements.txt`)
* Arduino Libraries
    * Adafruit [LED Backpack library](https://github.com/adafruit/Adafruit-LED-Backpack-Library/archive/master.zip)
    * Adafruit [GFX Library](https://github.com/adafruit/Adafruit-GFX-Library/archive/master.zip)
    * Adafruit [NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel/archive/master.zip)
* Leap Motion
    * [Desktop SDK](https://developer.leapmotion.com/v2)
    * Other libraries, as per https://api.leapmotion.com/documentation/python/devguide/Project_Setup.html

### .env
Add a `.env` file at the root, with the following variables (obviously, update to fit your setup and needs):
```
SERIAL_PORT="/dev/cu.usbmodem1411"
LEAP_SDK_LOCATION="/Users/moonshot/Documents/src/LeapSDK/lib"

LOAD_FRESH=True
CONNECT_TO_ARDUINO=True
LEAP_CONTROL=True
DEBUG=True
```

### Do tha thang
* Upload `bot/bot.ino` to your Arduino.
* Run `python run.py`

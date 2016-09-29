import os, time
import serial, struct

from dotenv import load_dotenv
dotenv_path = os.path.join(os.path.dirname(__file__), '.env')
load_dotenv(dotenv_path)

MOVES = ['r', 'p', 's']
INT_PLAY = {'r': 0, 'p': 1, 's': 2}
FULL_PLAY = {'r': 'rock', 'p': 'paper', 's': 'scissors'}

def main():
    try:
        bot = serial.Serial(os.environ.get("SERIAL_PORT"), 9600, timeout=1)

        while True:
            for move in MOVES:
                print FULL_PLAY[move]
                bot.write(struct.pack('>B', INT_PLAY[move]))
                time.sleep(2)
    except:
        raise
    finally:
        bot.write(struct.pack('>B', 3)) 
        bot.close()

if __name__ == "__main__":
    main()

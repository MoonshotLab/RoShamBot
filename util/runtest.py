#!/usr/bin/env python

from __future__ import division, print_function

import os, sys, inspect, tty, termios, time, cPickle, argparse, logging, subprocess

parser = argparse.ArgumentParser()
parser.add_argument('-p', '--port', dest='port', help='Arduino port', required=True)
args = parser.parse_args()

import serial
import struct

SERIAL_MAP = {
    'r': 0, 'p': 1, 's': 2, 'n': 3,
    'readScissors': 4, 'readPaper': 5, 'readRock': 6, 'readError': 7,
    'countOne': 8, 'countTwo': 9, 'countThree': 10, 'countThrow': 11,
    'incPlayerScore': 12, 'incBotScore': 13, 'resetScores': 14,
    'clearPlay': 15,
    'glowLoop': 16,
    'fillRingInc': 17, 'fillRingDec': 18,
    'botHandTest': 19,
    'playerWins': 20, 'botWins': 21,
    'playerVictory': 22, 'botVictory': 23,
    'playerWinRock': 24, 'playerTieRock': 25, 'playerLoseRock': 26,
    'playerWinPaper': 27, 'playerTiePaper': 28, 'playerLosePaper': 29,
    'playerWinScissors': 30, 'playerTieScissors': 31, 'playerLoseScissors': 32,
    'botWin': 33, 'botTie': 34, 'botLose': 35,
    'clearDisplay': 36,
    'reset': 37
}

def bot_write(msg):
    print(msg, SERIAL_MAP[msg])

    try:
        bot.write(struct.pack('>B', SERIAL_MAP[msg]))

    except:
        raise

def bot_write_raw(msg):
    print(msg)

    try:
        bot.write(struct.pack('>B', msg))

    except:
        raise


try:
    bot = serial.Serial(args.port, 9600, timeout=1)
except:
    print('Could not connect to Arduino.')
    logging.info('Could not connect to Arduino.')
    raise

def main():
    while True:
        msg = int(raw_input('Send: '))
        for key in SERIAL_MAP:
            if SERIAL_MAP[key] == msg:
                print(msg, key)
                bot_write_raw(msg)
                break



if __name__ == "__main__":
    main()

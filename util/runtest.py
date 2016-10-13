#!/usr/bin/env python

from __future__ import division, print_function

import os, sys, inspect, tty, termios, time, cPickle

from dotenv import load_dotenv
dotenv_path = os.path.join(os.path.dirname(__file__), '.env')
load_dotenv(dotenv_path)

src_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
lib_dir = os.path.abspath(os.path.join(src_dir, os.environ.get("LEAP_SDK_LOCATION")))
sys.path.insert(0, lib_dir)
import Leap

from numpy.random import choice as npchoice
from collections import deque
import serial
import struct

def str_to_bool(val): return val == 'True'

MEMORY = 5
ROUNDS_TO_WIN = 3
TIME_BETWEEN_MOVES = 2

LOAD_FRESH = str_to_bool(os.environ.get("LOAD_FRESH"))
CONNECT_TO_ARDUINO = str_to_bool(os.environ.get("CONNECT_TO_ARDUINO"))
PLAY_TUTORIAL = str_to_bool(os.environ.get("PLAY_TUTORIAL"))
DEBUG = str_to_bool(os.environ.get("DEBUG"))
REPLAY = str_to_bool(os.environ.get("REPLAY"))

PICKLE_FILE = 'model.pk'

ROOT = '0'
CHOICES = ['r', 'p', 's']
BEATS = {'r': 'p', 'p': 's', 's': 'r'}
FULL_PLAY = {'r': 'rock', 'p': 'paper', 's': 'scissors'}
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
    'botWin': 33, 'botTie': 34, 'botLose': 35
}

COUNTDOWN_MAP = {0: 'countOne', 1: 'countTwo', 2: 'countThree', 'throw': 'countThrow'}

class SampleListener(Leap.Listener):
    def getMove(self, controller):
        frame = controller.frame()

        max_num_frames = 10
        frame_num = 0

        while len(frame.hands) == 0 and frame_num < max_num_frames:
            frame = controller.frame()
            frame_num += 1

        if len(frame.hands) == 0:
            return False

        num_fingers = len(frame.fingers.extended())

        if num_fingers != 3:
            if num_fingers == 0:
                return 'r'
            elif num_fingers == 2:
                return 's'
            elif num_fingers in [1, 4, 5]:
                return 'p'
        else:
            return False

    # def on_frame(self, controller):
    #     global current_play
    #     frame = controller.frame()
    #
    #     if len(frame.hands):
    #         num_fingers = len(frame.fingers.extended())
    #
    #         if num_fingers in [0, 2, 4, 5]:
    #             if num_fingers == 0:
    #                 # rock
    #                 if current_play != 'rock':
    #                     bot_write('readRock')
    #                     current_play = 'rock'
    #             elif num_fingers == 2:
    #                 # scissors
    #                 if current_play != 'scissors':
    #                     bot_write('readScissors')
    #                     current_play = 'scissors'
    #             elif num_fingers in [4, 5]:
    #                 #paper
    #                 if current_play != 'paper':
    #                     bot_write('readPaper')
    #                     current_play = 'paper'
    #             elif current_play != False:
    #                 bot_write('readError')
    #                 current_play = False
    #         elif current_play != False:
    #             bot_write('readError')
    #             current_play = False
    #
    #     else:
    #         if current_play != None:
    #             bot_write('clearPlay')
    #             current_play = None

# ['p', 'r', 's'] => 'prs'
def concat_row(lst):
    return_string = ''

    for l in lst:
        try:
            return_string += str(l)
        except:
            pass

    return return_string

def get_guess(history, model):
    # initialize best guess dict
    guess_dict = {}
    for choice in CHOICES:
        guess_dict[choice] = 0

    lhistory = list(history)

    for query_level in range(len(lhistory) + 1):
        if query_level >= len(model): break

        query = get_concatted_history(lhistory, query_level)

        plays = get_possible_plays(query, model)
        for play in plays:
            guess_dict[play] += plays[play] * (query_level + 1)

    guess = dict_max(guess_dict)
    return guess

def str_to_bool(val):
    return val == 'True'

def get_possible_plays(query, model):
    plays = {}
    model_level = model[len(query)]

    for choice in CHOICES:
        if query + choice in model_level:
            plays[choice] = model_level[query + choice]

    return plays


def get_concatted_history(history, depth):
    if depth == 0:
        return ''

    else:
        return concat_row(history[0:depth])

def bot_write(msg):
    if DEBUG:
        print(msg, SERIAL_MAP[msg])

    try:
        bot.write(struct.pack('>B', SERIAL_MAP[msg]))

    except:
        raise

def bot_write_raw(msg):
    if DEBUG:
        print(msg)

    try:
        bot.write(struct.pack('>B', msg))

    except:
        raise

def dict_max(dict):
    best, best_val = '', 0

    for key in dict:
        if best == '' or dict[key] > best_val:
            best, best_val = key, dict[key]
        elif dict[key] == best_val:
            if isinstance(best, list):
                best.append(key)
            else:
                best = [best, key]

    if isinstance(best, list):
        return npchoice(best)

    if best == '':
        return CHOICES[0]

    return best

def get_game_result(p1, p2):
    if p1 == p2:
        return 0

    if p1 == BEATS[p2]:
        return 1

    return -1

def get_fresh_model():
    return {
        'record': {
            'win': 0,
            'loss': 0,
            'tie': 0,
            'games': 0
        },
        'nn': []
    }

def main():
    while True:
        msg = int(raw_input('Send: '))
        for key in SERIAL_MAP:
            if SERIAL_MAP[key] == msg:
                print(msg, key)
                bot_write_raw(msg)
                break

try:
    bot = serial.Serial(os.environ.get("SERIAL_PORT"), 9600, timeout=1)
except:
    print('Could not connect to Arduino.')
    raise

try:
    listener = SampleListener()
    controller = Leap.Controller()
    controller.add_listener(listener)
except:
    print('Could not connect to Leap controller.')
    raise


if LOAD_FRESH:
    M = get_fresh_model()
else:
    try:
        M = cPickle.load(open(PICKLE_FILE, 'rb'))
    except:
        print('Could not load pickled model. Starting fresh.')
        M = get_fresh_model()

current_play = None

if __name__ == "__main__":
    main()

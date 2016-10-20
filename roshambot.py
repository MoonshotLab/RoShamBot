#!/usr/bin/env python

from __future__ import division, print_function

import os, sys, inspect, tty, termios, time, cPickle, argparse, logging, subprocess

parser = argparse.ArgumentParser()
parser.add_argument('-p', '--port', dest='port', help='Arduino port', required=True)
parser.add_argument('-m', '--model', dest='model', help='Model path')
parser.add_argument('-l', '--logpath', dest='logpath', help='Log path')
parser.add_argument('-lf', '--loadfresh', dest='fresh', help='Load fresh')
parser.add_argument('-lp', '--leappath', dest='leappath', help='Lead SDK path', required=True)
args = parser.parse_args()

if args.logpath:
    logpath = args.logpath
else:
    logpath = 'roshambot.log'
logging.basicConfig(filename=logpath, level=logging.INFO)

src_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
lib_dir = os.path.abspath(os.path.join(src_dir, args.leappath))
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
TIMEOUT_LENGTH = 1000

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
    'botWin': 33, 'botTie': 34, 'botLose': 35,
    'clearDisplay': 36
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

def waitFor(thing, picky = False):
    """
    returns True if correct result was received within time limit
    else False
    """
    if not picky:
        something = "anything"

    print('waiting for ' + something)
    logging.info('waiting for ' + something)
    timeout_count = 0

    while True:
        if timeout_count >= TIMEOUT_LENGTH:
            return False

        bytes_to_read = bot.inWaiting()
        data = bot.read(bytes_to_read)

        if not picky or data == something:
            print('received data: ' + str(data))
            logging.info('received data: ' + str(data))
            return True

        timeout_count += 1
        time.sleep(0.1)

def waitForSomething(something):
    waitFor(something, True)

def waitForAnything():
    waitFor(_, False)

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

    guess = weighted_random_dict_choice(guess_dict)
    return guess

def str_to_bool(val):
    return val == 'True'

def playsound(file):
    subprocess.call(['afplay', file])

def short_beep():
    playsound('assets/shortbeep.wav')

def long_beep():
    playsound('assets/longbeep.wav')

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

def weighted_random_dict_choice(dict):
    try:
        # make sure dict has all possible choices
        total_weight = 0.0 # float

        for key in dict:
            total_weight += dict[key]

        # return a random weighted choice
        return npchoice(dict.keys(), p=[dict[key]/total_weight for key in dict.keys()])
    except:
        # default to a random choice
        return npchoice(CHOICES)

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

try:
    if str_to_bool(args.fresh):
        LOAD_FRESH = True
    else:
        LOAD_FRESH = False
except:
    LOAD_FRESH = False

try:
    if args.model:
        PICKLE_FILE = args.model
    else:
        PICKLE_FILE = 'model.pk'
except:
    PICKLE_FILE = 'model.pk'


try:
    bot = serial.Serial(args.port, 9600, timeout=1)
except:
    print('Could not connect to Arduino.')
    logging.info('Could not connect to Arduino.')
    raise

try:
    listener = SampleListener()
    controller = Leap.Controller()
    controller.add_listener(listener)
except:
    print('Could not connect to Leap controller.')
    logging.info('Could not connect to Leap controller.')
    raise


if LOAD_FRESH:
    M = get_fresh_model()
else:
    try:
        M = cPickle.load(open(PICKLE_FILE, 'rb'))
    except:
        print('Could not load pickled model. Starting fresh.')
        logging.info('Could not load pickled model. Starting fresh.')
        M = get_fresh_model()

current_play = None

def main():
    try:
        # can't stop won't stop
        while True:
            bot_write('clearPlay')

            # enticing glow
            current_mode = 0
            bot_write('glowLoop')

            # user hand -> fill ring -> start game
            ready_count = 0
            ready_limit = 20
            while True:
                if ready_count >= ready_limit:
                    break

                frame = controller.frame()

                sleep_timing = 0.1

                if len(frame.hands) > 0:
                    if current_mode != 1:
                        current_mode = 1

                    bot_write('fillRingInc')
                    ready_count += 1
                    time.sleep(sleep_timing)
                else:
                    if ready_count > 0:
                        bot_write('fillRingDec')
                        ready_count -= 1
                        time.sleep(sleep_timing)

            bot_write('botHandTest')

            # reset game vars
            game = {}
            for key in ['win', 'tie', 'loss']:
                game[key] = 0
            game['turn'] = 1
            game['history'] = deque()
            player_move = False
            invalid_play_count = 0

            bot_write('resetScores')

            # game loop
            while True:
                time.sleep(TIME_BETWEEN_MOVES)

                # neutral bot pose
                bot_write('n')
                bot_write('clearPlay')

                # wait for start from arduino
                waitResult = waitForSomething("wipeDone")

                print('postwait')
                logging.info('postwait')

                if not waitResult:
                    print('timeout')
                    logging.info('timeout')
                    break # restart

                # traverse history, updating weights (only if last game was not tie)
                if len(game['history']) > 0 and player_move != False:
                    len_history = len(game['history'])
                    nodes = list(game['history'])

                    # build up neural net if depth not present
                    if len_history > len(M['nn']):
                        M['nn'].append(dict())

                    while len(nodes):
                        depth = len(nodes) - 1 # 1-based -> 0-based

                        concatted_row = concat_row(nodes)
                        concatted_row = concatted_row[::-1] # look backwards in history for most likely next play

                        if concatted_row in M['nn'][depth]:
                            print('incrementing: ', concatted_row, ' to a val of ', M['nn'][depth][concatted_row])
                            M['nn'][depth][concatted_row] += 1
                        else:
                            print('adding: ', concatted_row)
                            M['nn'][depth][concatted_row] = 1

                        nodes.pop() # pop off

                # come up with our play
                guess = get_guess(game['history'], M['nn']) # what we guess they'll play
                bot_move = BEATS[guess] # what'll beat it

                # countdown
                for i in range(3):
                    bot_write(COUNTDOWN_MAP[i])
                    short_beep()
                    time.sleep(TIME_BETWEEN_MOVES / 3.0)

                    # wait for start from arduino
                    waitResult = waitForAnything()

                    print('postwait')
                    logging.info('postwait')

                    if not waitResult:
                        print('timeout')
                        logging.info('timeout')
                        break # restart

                # throw
                bot_write(COUNTDOWN_MAP['throw'])
                long_beep()

                # wait for start from arduino
                waitResult = waitForSomething("throwDone")

                print('postwait')
                logging.info('postwait')

                if not waitResult:
                    print('timeout')
                    logging.info('timeout')
                    break # restart

                # use a move dict to get an average to make sure we're reading the input correctly
                move_history = {} # reset dict
                for i in range(50):
                    move = listener.getMove(controller)
                    if move:
                        if move in move_history:
                            move_history[move] += 1
                        else:
                            move_history[move] = 1

                # no input
                if not len(move_history):
                    bot_write('readError')
                    player_move = False
                    invalid_play_count += 1

                    # wait for start from arduino
                    waitResult = waitForSomething("errorDone")

                    print('postwait')
                    logging.info('postwait')

                    if not waitResult:
                        print('timeout')
                        logging.info('timeout')
                        break # restart

                    if invalid_play_count >= 5:
                        break

                    continue

                invalid_play_count = 0 # reset

                # most frequent input
                player_move = dict_max(move_history)

                print('player played ' + player_move)
                logging.info('player played ' + player_move)
                print('bot played ' + bot_move)
                logging.info('bot played ' + bot_move)

                # play bot move
                bot_write(bot_move)

                # wait for start from arduino
                waitResult = waitForSomething("moveDone")

                print('postwait')
                logging.info('postwait')

                if not waitResult:
                    print('timeout')
                    logging.info('timeout')
                    break # restart

                # increment total number of games played by this model
                M['record']['games'] += 1

                # see who won
                game_result = get_game_result(bot_move, player_move)

                if game_result == 0: # tie
                    game['tie'] += 1
                    M['record']['tie'] += 1

                    bot_write('clearPlay')
                    bot_write('playerTie' + FULL_PLAY[player_move].capitalize())
                    bot_write('botTie')
                elif game_result == -1: # bot loses / player wins
                    game['loss'] += 1
                    M['record']['loss'] += 1

                    bot_write('clearPlay')
                    bot_write('playerWin' + FULL_PLAY[player_move].capitalize())
                    bot_write('incPlayerScore')
                    bot_write('botLose')
                elif game_result == 1: # bot wins / player loses
                    game['win'] += 1
                    M['record']['win'] += 1

                    bot_write('clearPlay')
                    bot_write('playerLose' + FULL_PLAY[player_move].capitalize())
                    bot_write('incBotScore')
                    bot_write('botWin')

                # wait for start from arduino
                waitResult = waitForSomething("botResultDone")

                print('postwait')
                logging.info('postwait')

                if not waitResult:
                    print('timeout')
                    logging.info('timeout')
                    break # restart

                if game['loss'] >= ROUNDS_TO_WIN or game['win'] >= ROUNDS_TO_WIN:
                    print(M['record'])
                    for model_level in M['nn']:
                        print(model_level)

                    # flash winner
                    if game['loss'] >= ROUNDS_TO_WIN:
                        bot_write('playerVictory')

                    if game['win'] >= ROUNDS_TO_WIN:
                        bot_write('botVictory')

                    # wait for start from arduino
                    waitResult = waitForSomething("victoryDone")

                    print('postwait')
                    logging.info('postwait')

                    break

                game['history'].appendleft(player_move)

                while len(game['history']) > MEMORY:
                    game['history'].pop()

                game['turn'] += 1
            # break
    except:
        raise
    finally:
        try:
            controller.remove_listener(listener)
        except:
            pass

        cPickle.dump(M, open(PICKLE_FILE, 'wb'))


if __name__ == "__main__":
    main()

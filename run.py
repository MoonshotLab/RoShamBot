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
INITIAL_WEIGHT = 1
ROUNDS_TO_WIN = 1
TIME_BETWEEN_MOVES = 2.5

LOAD_FRESH = str_to_bool(os.environ.get("LOAD_FRESH"))
CONNECT_TO_ARDUINO = str_to_bool(os.environ.get("CONNECT_TO_ARDUINO"))
PLAY_TUTORIAL = str_to_bool(os.environ.get("PLAY_TUTORIAL"))
LEAP_CONTROL = str_to_bool(os.environ.get("LEAP_CONTROL"))
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
    'incPlayerScore': 12, 'incBotScore': 13, 'resetScores': 14
}
DIVIDER = "=" * 80
COUNTDOWN_MAP = {1: 'countOne', 2: 'countTwo', 3: 'countThree', 'throw': 'countThrow'}

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

    def on_frame(self, controller):
        frame = controller.frame()

        if frame.id % 15 == 0 and len(frame.hands):
            num_fingers = len(frame.fingers.extended())

            if num_fingers in [0, 2, 4, 5]:
                if num_fingers == 0:
                    maybe_write('readRock')
                    # time.sleep(100)
                elif num_fingers == 2:
                    maybe_write('readScissors')
                    # time.sleep(100)
                elif num_fingers in [4, 5]:
                    maybe_write('readPaper')
                    # time.sleep(100)

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
    # print(lhistory)
    # print(model)

    for query_level in range(len(lhistory) + 1):
        if query_level >= len(model): break

        query = get_concatted_history(lhistory, query_level)
        # print('query: ', query)

        plays = get_possible_plays(query, model)
        # print('plays: ', plays)
        for play in plays:
            guess_dict[play] += plays[play] * (query_level + 1)
            # print(play, guess_dict[play])

    # print(guess_dict)
    guess = dict_max(guess_dict)
    # print(guess)
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

def maybe_sleep(duration):
    if LEAP_CONTROL:
        time.sleep(duration)

def maybe_write(msg):
    print(msg, SERIAL_MAP[msg], BOT_CONNECTED)

    try:
        if BOT_CONNECTED:
            bot.write(struct.pack('>B', SERIAL_MAP[msg]))
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
        # print('bar')
        return npchoice(best)

    if best == '':
        # print('foo')
        return CHOICES[0]

    # print('baz')
    return best

def get_game_result(p1, p2):
    if p1 == p2:
        return 0

    if p1 == BEATS[p2]:
        return 1

    return -1


def print_divider():
    print()
    print(DIVIDER)
    print()

def print_results(game):
    print_divider()
    print("You won %.2f%% (%d / %d)" % (game['loss'] / game['turn'] * 100, game['loss'], game['turn']))
    print("We tied %.2f%% (%d / %d)" % (game['tie'] / game['turn'] * 100, game['tie'], game['turn']))
    print("You lost %.2f%% (%d / %d)" % (game['win'] / game['turn'] * 100, game['win'], game['turn']))
    print_divider()

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

if CONNECT_TO_ARDUINO:
    try:
        bot = serial.Serial(os.environ.get("SERIAL_PORT"), 9600, timeout=1)
        BOT_CONNECTED = True
    except:
        print('Could not connect to Arduino.')
        BOT_CONNECTED = False
else:
    BOT_CONNECTED = False

if LEAP_CONTROL:
    try:
        listener = SampleListener()
        controller = Leap.Controller()
        controller.add_listener(listener)
        leap_connected = True
    except:
        print('Could not connect to Leap controller.')
        leap_connected = False
else:
    leap_connected = False

if LOAD_FRESH:
    M = get_fresh_model()
else:
    try:
        M = cPickle.load(open(PICKLE_FILE, 'rb'))
    except:
        print('Could not load pickled model. Starting fresh.')
        M = get_fresh_model()

def mainBak():
    try:
        first_run = True

        history = deque()
        char_map = {'left': 'r', 'up': 'p', 'right': 's'}

        while True:
            if not first_run:
                print()
                print()
                print()
                print()
                print()
                print()
                print()
                print()
                maybe_sleep(TIME_BETWEEN_MOVES)

            game = {}
            for key in ['win', 'tie', 'loss']:
                game[key] = 0
            game['turn'] = 1

            maybe_write('resetScores')

            print(DIVIDER)
            print()
            print('Welcome to Roshambot 3000')
            print_divider()

            maybe_sleep(0.5)


            """
            WAIT TO BEGIN
            """
            if leap_connected:
                print ('Hold your hand over the screen to begin.')
                ready_frame_count = 0
                while True:
                    if ready_frame_count >= 20:
                        break

                    frame = controller.frame()

                    if len(frame.hands) > 0:
                        ready_frame_count += 1
                        sys.stdout.write('.')
                        sys.stdout.flush()
                        maybe_sleep(0.05)

            print()
            maybe_sleep(0.5)


            """
            TUTORIAL
            """
            if PLAY_TUTORIAL and leap_connected:
                print()
                print("Let's run through a tutorial.")
                maybe_sleep(1)
                print("Hold your hand over the input device to play.")
                maybe_sleep(1)
                print("We'll count down from 3, and on 'THROW', play your move.")
                print()
                maybe_sleep(2)

                print ('Hold your hand over the screen when ready...')
                # print()
                ready_frame_count = 0
                while True:
                    if ready_frame_count >= 20:
                        print()
                        break

                    frame = controller.frame()

                    if len(frame.hands) > 0:
                        ready_frame_count += 1
                        sys.stdout.write('.')
                        sys.stdout.flush()
                        maybe_sleep(0.05)

                print()
                tutorial_repeat = 1
                tutorial_moves = [choice for choice in CHOICES * tutorial_repeat]
                maybe_sleep(1)

                for tutorial_move in tutorial_moves:
                    while True:
                        print_divider()
                        print("On 3, throw " + FULL_PLAY[tutorial_move] + ".")
                        print()

                        maybe_write('n')
                        time.sleep(1)

                        for i in range(3, 0, -1):
                            print(str(i) + '... ')
                            maybe_write(COUNTDOWN_MAP[i])
                            maybe_sleep(0.9)

                        print('THROW')
                        print()

                        maybe_write(COUNTDOWN_MAP['throw'])
                        maybe_write(tutorial_move)

                        # use a move dict to get an average to make sure we're reading the input correctly
                        move_history = {} # reset dict
                        for i in range(20):
                            move = listener.getMove(controller)
                            if move:
                                if move in move_history:
                                    move_history[move] += 1
                                else:
                                    move_history[move] = 1

                        if not len(move_history):
                            print("Could not read input, let's again...")
                            maybe_sleep(TIME_BETWEEN_MOVES)
                            print()

                            continue

                        their_play = dict_max(move_history)

                        if their_play == tutorial_move:
                            print('Great!')
                            maybe_sleep(TIME_BETWEEN_MOVES)
                            break
                        else:
                            print("Sorry, I thought you played " + FULL_PLAY[their_play] + ". Let's try again.")
                            maybe_sleep(TIME_BETWEEN_MOVES)
                            continue
                        print_divider()

            print()
            print("Now, let's play!")
            time.sleep(1)
            print("We'll play best of %d." % ROUNDS_TO_WIN)
            time.sleep(1)
            print("On the count of 3, throw your move!")
            time.sleep(1)
            print("Good luck!")
            time.sleep(2)
            print()
            if leap_connected:
                print ('Hold your hand over the screen when ready...')
                ready_frame_count = 0
                while True:
                    if ready_frame_count >= 20:
                        break

                    frame = controller.frame()

                    if len(frame.hands) > 0:
                        ready_frame_count += 1
                        sys.stdout.write('.')
                        sys.stdout.flush()
                        maybe_sleep(0.05)
            else:
                raw_input("Press Enter to continue.")

            maybe_sleep(0.5)
            print()


            """
            LET'S PLAY
            """
            while True:

                maybe_write('n')

                # traverse history, updating weights (only if last game was not tie)
                try:
                    if len(history) > 0:
                        len_history = len(history)
                        nodes = list(history)

                        if len_history > len(M['nn']):
                            M['nn'].append(dict())

                        while len(nodes):
                            # print(nodes)
                            depth = len(nodes) - 1 # 1-based -> 0-based
                            concatted_row = concat_row(nodes)
                            concatted_row = concatted_row[::-1] # look backwards in history for most likely next play

                            if concatted_row in M['nn'][depth]:
                                if DEBUG:
                                    print('incrementing: ', concatted_row, ' to a val of ', M['nn'][depth][concatted_row])
                                M['nn'][depth][concatted_row] += 1
                            else:
                                if DEBUG:
                                    print('adding: ', concatted_row)
                                M['nn'][depth][concatted_row] = 1

                            nodes.pop()
                except:
                    pass

                print_divider()
                guess = get_guess(history, M['nn'])
                our_play = BEATS[guess]

                print("ROUND %d" % game['turn'])
                time.sleep(1)

                if leap_connected:
                    for i in range(3, 0, -1):
                        print(str(i) + '... ')
                        maybe_write(COUNTDOWN_MAP[i])
                        maybe_sleep(0.9)

                    print('THROW')
                    maybe_write(COUNTDOWN_MAP['throw'])
                    print()

                    # use a move dict to get an average to make sure we're reading the input correctly
                    move_history = {} # reset dict
                    for i in range(20):
                        move = listener.getMove(controller)
                        if move:
                            if move in move_history:
                                move_history[move] += 1
                            else:
                                move_history[move] = 1

                    if not len(move_history):
                        print('Could not read input, trying again...')
                        maybe_write('readError')
                        print_divider()
                        maybe_sleep(TIME_BETWEEN_MOVES)
                        print()
                        continue

                    their_play = dict_max(move_history)
                else:
                    char = get_char()
                    their_play = char_map[char]

                    if char == "down" or char == False:
                        break
                # print(their_play)
                # print(move_history)


                print("I guessed you'd play %s so I'll play %s" % (FULL_PLAY[guess].upper(), FULL_PLAY[our_play].upper()))
                print(ASCII_ART[our_play])
                print()

                maybe_write(our_play)

                M['record']['games'] += 1
                game_result = get_game_result(our_play, their_play)

                print("I believe you played %s" % (FULL_PLAY[their_play].upper()))
                maybe_write('read' + FULL_PLAY[their_play].capitalize())

                if game_result == 0:
                    print('Tie!')
                    game['tie'] += 1
                    M['record']['tie'] += 1
                elif game_result == -1:
                    print('You win!')
                    game['loss'] += 1
                    maybe_write('incPlayerScore')
                    M['record']['loss'] += 1
                elif game_result == 1:
                    print('I win!')
                    game['win'] += 1
                    maybe_write('incBotScore')
                    M['record']['win'] += 1

                print("Out of %d games, you've won %d, I've won %d, and we've tied %d times." % (game['turn'], game['loss'], game['win'], game['tie']))
                print_divider()

                if game['loss'] >= ROUNDS_TO_WIN or game['win'] >= ROUNDS_TO_WIN:
                # if game['turn'] >= ROUNDS_TO_WIN:
                    maybe_sleep(TIME_BETWEEN_MOVES)
                    print_results(game)

                    if DEBUG:
                        print(M['record'])
                        for model_level in M['nn']:
                            print(model_level)

                    if REPLAY:
                        first_run = False

                    break

                history.appendleft(their_play)

                while len(history) > MEMORY:
                    history.pop()

                game['turn'] += 1
                maybe_sleep(TIME_BETWEEN_MOVES)
                print()

    except:
        raise
        maybe_sleep(TIME_BETWEEN_MOVES)
        if game['turn'] > 1:
            print_results(game)

    finally:
        print("Thanks for playing!")
        print_divider()
        if leap_connected:
            controller.remove_listener(listener)

        # pickle graph
        cPickle.dump(M, open(PICKLE_FILE, 'wb'))

def main():
    try:
        sys.stdin.readline()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()

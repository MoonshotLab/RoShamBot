#!/usr/bin/env python

from __future__ import division, print_function

import os, sys, inspect, tty, termios, time, cPickle

PATH_TO_LEAP_SDK = '/Users/prichey/Documents/src/LeapSDK/lib'
src_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
lib_dir = os.path.abspath(os.path.join(src_dir, PATH_TO_LEAP_SDK))
sys.path.insert(0, lib_dir)
import Leap

from numpy.random import choice as npchoice
from collections import deque
import serial
import struct

MEMORY = 5

LOAD_FRESH = True

PICKLE_FILE = 'model_list.pk'

ROOT = '0'
CHOICES = ['r', 'p', 's']
INITIAL_WEIGHT = 1
BEATS = {'r': 'p', 'p': 's', 's': 'r'}
FULL_PLAY = {'r': 'rock', 'p': 'paper', 's': 'scissors'}
INT_PLAY = {'r': 0, 'p': 1, 's': 2}
ROUNDS_TO_WIN = 5

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

def get_possible_plays(query, model):
    plays = {}
    model_level = model[len(query)]

    for choice in CHOICES:
        if query + choice in model_level:
            plays[choice] = model_level[query + choice]

    # print(plays)
    return plays


def get_concatted_history(history, depth):
    if depth == 0:
        return ''

    else:
        return concat_row(history[0:depth])



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

def main():
    try:

        # try:
        #     bot = serial.Serial('/dev/cu.usbmodem1411', 9600, timeout=1)
        # except:
        #     print('Could not connect to Arduino.')
        #     return

        try:
            listener = SampleListener()
            controller = Leap.Controller()
            controller.add_listener(listener)
        except:
            print('Could not connect to Leap controller.')
            return

        if LOAD_FRESH:
            M = []
        else:
            try:
                M = cPickle.load(open(PICKLE_FILE, 'rb'))
            except:
                print('Could not load pickled model. Starting fresh.')
                M = []


        history = deque()
        char_map = {'left': 'r', 'up': 'p', 'right': 's'}

        print('Welcome to Roshambot 3000')
        # time.sleep(0.5)
        print("We'll play best of %d" % ROUNDS_TO_WIN)
        # time.sleep(0.5)
        print()
        time.sleep(0.5)

        raw_input("Press Enter to continue...")

        # char = get_char()

        game = {}
        for key in ['turn', 'win', 'tie', 'loss']:
            game[key] = 0

        while True:

            game['turn'] += 1
            # traverse history, updating weights
            len_history = len(history)
            nodes = list(history)

            if len_history > len(M):
                M.append(dict())


            while len(nodes):
                # print(nodes)
                depth = len(nodes) - 1 # 1-based -> 0-based
                concatted_row = concat_row(nodes)
                concatted_row = concatted_row[::-1] # look backwards in history for most likely next play

                if concatted_row in M[depth]:
                    # print('incrementing: ', concatted_row, ' to a val of ', M[depth][concatted_row])
                    M[depth][concatted_row] += 1
                else:
                    # print('adding: ', concatted_row)
                    M[depth][concatted_row] = 1

                nodes.pop()


            guess = get_guess(history, M)
            our_play = BEATS[guess]

            for i in range(3, 0, -1):
                print(str(i) + '... ')
                time.sleep(0.9)
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
                time.sleep(2)
                continue
            else:
                their_play = dict_max(move_history)
                # print(their_play)
                # print(move_history)


            print("I guessed you'd play %s so I'll play %s" % (FULL_PLAY[guess].upper(), FULL_PLAY[our_play].upper()))
            # bot.write(struct.pack('>B', INT_PLAY[our_play]))

            game_result = get_game_result(our_play, their_play)

            print("I believe you played %s" % (FULL_PLAY[their_play].upper()))
            if game_result == 0:
                print('Tie!')
                game['tie'] += 1
            elif game_result == -1:
                print('You win!')
                game['loss'] += 1
            elif game_result == 1:
                print('I win!')
                game['win'] += 1

            print()
            if game['loss'] >= ROUNDS_TO_WIN or game['win'] >= ROUNDS_TO_WIN:
                print("You won %.2f%% (%d / %d)" % (game['loss'] / game['turn'] * 100, game['loss'], game['turn']))
                print("We tied %.2f%% (%d / %d)" % (game['tie'] / game['turn'] * 100, game['tie'], game['turn']))
                print("You lost %.2f%% (%d / %d)" % (game['win'] / game['turn'] * 100, game['win'], game['turn']))
                for model_level in M:
                    print(model_level)
                break

            history.appendleft(their_play)

            while len(history) > MEMORY:
                history.pop()

            time.sleep(2)
            print()

    except:
        print("You won %.2f%% (%d / %d)" % (game['loss'] / game['turn'] * 100, game['loss'], game['turn']))
        print("We tied %.2f%% (%d / %d)" % (game['tie'] / game['turn'] * 100, game['tie'], game['turn']))
        print("You lost %.2f%% (%d / %d)" % (game['win'] / game['turn'] * 100, game['win'], game['turn']))

    finally:
        # pickle graph
        cPickle.dump(M, open(PICKLE_FILE, 'wb'))


if __name__ == "__main__":
    main()

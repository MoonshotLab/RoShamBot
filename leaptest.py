import os, sys, inspect, time, cPickle
src_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
lib_dir = os.path.abspath(os.path.join(src_dir, '/Users/prichey/Documents/src/LeapSDK/lib'))
sys.path.insert(0, lib_dir)
import Leap

CHOICES = ['r', 'p', 's']

class SampleListener(Leap.Listener):

    def on_connect(self, controller):
        print "Connected"


    def on_frame1(self, controller):
        frame = controller.frame()

        if len(frame.hands) != 1:
            return

        num_fingers = len(frame.fingers.extended())

        if num_fingers in [0, 2, 4, 5]:
            if num_fingers == 0:
                print 'ROCK'
            elif num_fingers == 2:
                print 'SCISSORS'
            elif num_fingers in [4, 5]:
                print 'PAPER'
        else:
            print 'INVALID'

        # print "Frame id: %d, timestamp: %d, hands: %d, fingers extended: %d" % (
        #   frame.id, frame.timestamp, len(frame.hands), len(frame.fingers.extended()))

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
        print num_fingers

        if num_fingers != 3:
            if num_fingers == 0:
                return 'r'
            elif num_fingers == 2:
                return 's'
            elif num_fingers in [1, 4, 5]:
                return 'p'
        else:
            return False


def main():
    # Create a sample listener and controller
    listener = SampleListener()
    controller = Leap.Controller()

    # Have the sample listener receive events from the controller
    controller.add_listener(listener)

    print 'Get ready...'
    time.sleep(0.5)

    while True:
        try:
            for i in range(3, 0, -1):
                print i
                time.sleep(0.9)

            # use a move dict to get an average over ten frames to make sure we're guessing the right move
            # reset dict
            move_history = {}

            for i in range(10):
                move = listener.getMove(controller)
                if move:
                    if move in move_history:
                        move_history[move] += 1
                    else:
                        move_history[move] = 1

            if not len(move_history):
                print 'Could not read input, trying again...'
            else:
                # move = dict_max(move_history)
                print move_history

            time.sleep(1.5)
            print
            print 'NEW GAME'
        except KeyboardInterrupt:
            break

    controller.remove_listener(listener)

if __name__ == "__main__":
    main()

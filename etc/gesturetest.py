#!/usr/bin/env python

import os, sys, inspect

from dotenv import load_dotenv
dotenv_path = os.path.join(os.path.dirname(__file__), '.env')
load_dotenv(dotenv_path)

src_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
lib_dir = os.path.abspath(os.path.join(src_dir, os.environ.get("LEAP_SDK_LOCATION")))
sys.path.insert(0, lib_dir)
import Leap

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

        # print "Frame id: %d, timestamp: %d, hands: %d, fingers: %d" % (
        #       frame.id, frame.timestamp, len(frame.hands), len(frame.fingers))

        for gesture in frame.gestures():
            if gesture.type is Leap.Gesture.TYPE_SWIPE:
                print 'SWIPE' + str(gesture.id)

def main():
    listener = SampleListener()
    controller = Leap.Controller()
    controller.add_listener(listener)
    controller.enable_gesture(Leap.Gesture.TYPE_SWIPE)

    try:
        sys.stdin.readline()
    except KeyboardInterrupt:
        pass
    finally:
        # Remove the sample listener when done
        controller.remove_listener(listener)

if __name__ == "__main__":
    main()

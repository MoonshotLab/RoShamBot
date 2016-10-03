from __future__ import print_function
import time
import sys


for j in range(5):
    for i in range(5):
        sys.stdout.write('.')
        sys.stdout.flush()
        time.sleep(0.1)

    # sys.stdout.write("\033[F") #back to previous line
    sys.stdout.write("\033[K") #clear line
    print()

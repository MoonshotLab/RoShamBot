import subprocess, time

for i in range(3):
    subprocess.call(['afplay', "assets/shortbeep.wav"])
    time.sleep(1)

subprocess.call(['afplay', "assets/longbeep.wav"])

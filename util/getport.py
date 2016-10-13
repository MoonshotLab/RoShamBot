import sys
import json

def get_port(json_in):
    data = json.loads(json_in)
    for entry in data:
        if 'USB' in entry['hwid']:
            print entry['port']

data = sys.stdin.read()
get_port(data)

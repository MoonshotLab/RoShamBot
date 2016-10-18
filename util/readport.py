import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-p', '--port', dest='port', help='Arduino port')
args = parser.parse_args()

if args.port:
    print 'port: ' + args.port

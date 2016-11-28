# helper to mess with model outside of game loop
import cPickle

PICKLE_FILE = 'model.pk'

M = cPickle.load(open(PICKLE_FILE, 'rb'))

print M

cPickle.dump(M, open(PICKLE_FILE, 'wb'))

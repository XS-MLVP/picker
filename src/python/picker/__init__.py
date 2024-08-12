
import os
import sys

def cmd():
    picker = os.path.join(os.path.dirname(os.path.realpath(__file__)), "bin/picker")
    sys.argv[0] = picker
    os.system(" ".join(sys.argv))

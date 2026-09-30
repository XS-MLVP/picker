
import os
import sys

def cmd():
    picker = os.path.join(os.path.dirname(os.path.realpath(__file__)), "bin/picker")
    os.execv(picker, [picker, *sys.argv[1:]])

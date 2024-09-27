import sys
sys.path.append('../')
from xsp_seq_xagent import *

if __name__ == "__main__":

    def receive_sequence(message):
        sequence = adder_trans(message)
        print("python receive sequence",sequence.a,sequence.b)
        
    env = Env("adder_trans")
    sequence = adder_trans()

    for i in range(10):
        sequence.a.value = i
        sequence.b.value = i + 1
        sequence.sum.value = sequence.a.value + sequence.b.value
    
    env.run(30)

import sys
sys.path.append('../')
from adder_trans.adder_trans_xagent import *

if __name__ == "__main__":

    def receive_sequence(message):
        sequence = adder_trans(message)
        print("python receive sequence",sequence.a,sequence.b)
        
    agent = Agent("","adder_trans",receive_sequence)
    
    agent.run(100)

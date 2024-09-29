import sys
sys.path.append('../')
from adder_trans.adder_trans_xagent import *

if __name__ == "__main__":

    agent = Agent("adder_trans")
    
    for i in range(10):
        sequence = adder_trans()
        sequence.a.value = i
        sequence.b.value = i + 1
        sequence.send(agent)
        agent.run(1)


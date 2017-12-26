# brainthread
A brainfuck derivative interpreter

Features:
* Runs Brainfuck, pBrain, Brainfork and Brainthread code
* Cells can be either 8, 16 or 32 bits in size
* Memory tape can grow automatically or be looped


Brainthread:
* is Brainfuck compatible
* has functions from pBrain (function call operand is *, not :)
* has threading from Brainfork: { 'fork' inhanced by control commands } 'join' and ! 'terminate' 
* has heaps: & 'push', ^ 'pop' and % 'swap'. The heap command preceded by ~ causes the shared heap to be used. Threads can commnicate this way.
* introduces decimal input and output (; and : commands)







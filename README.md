# brainthread
A brainfuck derivative interpreter

Features:
* Runs Brainfuck, pBrain, Brainfork and Brainthread code
* Cells can be either 8, 16 or 32 bits in size
* Memory tape can grow automatically or be looped
* Can parse and analyse the code for flaws


Brainthread language:
* is Brainfuck compatible
* has functions from pBrain (function call command is *, not : )
* has threading from Brainfork: { 'fork' inhanced by control commands } 'join' and ! 'terminate' 
* has heaps: the command & is 'push', ^ 'pop' and % 'swap'. A heap command preceded by ~ causes the shared heap to be used. Threads can commnicate this way.
* introduces integer input and output ( ; and : commands)







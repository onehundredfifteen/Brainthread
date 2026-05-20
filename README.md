# brainthread
***A brainfuck derivative interpreter***

Features:
* Runs **Brainfuck**, pBrain, Brainfork and *Brainthread* code
* **Interactive mode**
* Cells can be either 8, 16 or 32 bits in size
* Memory tape can grow automatically or be looped
* Can parse and **analyze** the code for flaws

# Brainthread language
* is Brainfuck compatible
* has functions from pBrain (function call command is __*__, not __:__)
* has threading from Brainfork: __{__ 'fork' enhanced by control command __}__ 'join'.
Waits for all children threads. If the current cell is equal to zero, terminated in a non-blocking way.
* has a thread-safe stack: the command __!__ is 'push', __^__ 'pop' and __%__ 'swap'. Threads can commnicate this way.
* introduces integer input and output (__;__ and __:__ commands)

## Legacy
Current Brainthread version is 2.0
* Removed additional stack per thread
* Command __!__ now means push, not terminate
* Command __}__ now terminates a thread if current cell is equal to 0
* Command __~__ loses sense and it is removed with __&__

# More about the Analyzer & Optimizer
 The Analyzer can perform various tests on the code to identify potential issues and optimizations such as:

* Testing for infinite loops, redundant moves, repetition, and loop performance.
* Counting the number of function calls, functions defined, forks, joins, 
and more. *brainthread, pBrain only.*
* Performing tests specific to certain instruction types like flow-changing instructions repetition optimizable operators, arithmetic instructions, move instructions, linkable instructions, etc.

The **Optimizer** can wrap up repetitions (4 consecutive plus commands ++++ use 1 cycle to add 4)
and interptering '[-]' as ':=0'. 

Saving loop positions is default and always done. However optimiser itself needs to be turned on.

Only one level of optimization is available.
# brainthread
***A brainfuck derivative interpreter***

Features:
* **Brainfuck**, pBrain, Brainfork and Brainthread code
* **Interactive mode**
* Cells can be either 8, 16 or 32 bits in size
* Memory tape can grow automatically or be looped
* Can parse and **analyze** the code for flaws

# Brainthread language:
* is Brainfuck compatible
* has functions from pBrain (function call command is *, not : )
* has threading from Brainfork: { 'fork' inhanced by control commands } 'join' and ! 'terminate' 
* has heaps: the command & is 'push', ^ 'pop' and % 'swap'. A heap command preceded by ~ causes the shared heap to be used. Threads can commnicate this way.
* introduces integer input and output ( ; and : commands)

## More about the Analyzer & Optimizer
 The Analyzer can perform various tests on the code to identify potential issues and optimizations such as:

* Testing for infinite loops, redundant moves, repetition, and loop performance.
* Counting the number of function calls, functions defined, forks, joins, 
and more. *brainthread, pBrain only.*
* Performing tests specific to certain instruction types like flow-changing instructions repetition optimizable operators, arithmetic instructions, move instructions, linkable instructions, etc.

The **Optimizer** can wrap up repetitions (4 consecutive plus commands ++++ use 1 cycle to add 4)
and interptering '[-]' as ':=0'. 
Saving loop positions is default and always done. However optimiser itself need to be truned on.
Only one level of optimization is available.






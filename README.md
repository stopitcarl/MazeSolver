# MazeSolver

## COMPILATION:	
To compile both solutions at once run './compileAll.sh'. To compile each individually, run 'make' in any of the CircuitRouter directories.
	
## USAGE:
There are two Solvers, a SeqSolver, who solves routes all sequencially and ParSolver who used multithreading to solve the maze as fast as possible. Both solvers write the solved maze to a file with .res extension.		
### SeqSolver
CircuiteRouter-SeqSolver receives an input file and some optional arguments.
	e.g. "./CircuitRouter-SeqSolver test.in" will write the solved maze file to test.in.res
### ParSolver
CircuitRouter-ParSolver receives an input file and a positive number of threads.
	e.g. "./CircuitRouter-ParSolver -t 8 test.in" will use 8 threads and write the solved maze file to test.in.res
### SimpleShell
CircuitRouter-SimpleShell receives an optional argument of maximum number of active forks, if none is given, there is no maximum. It creates a shell that receives commands. To solve an inputfile type 'run INPUTFILE' and if the maximum active children hasnt been reached it will fork to solve that inputfile with SeqSolver, otherwise, it will wait untill a child dies to fork again. If 'exit' is typed, it prints the number of children it created throughout execution and their exit status.
### AdvShell and Client
CircuitRouter-AdvShell behaves just like SimpleShell but will also open a named pipe (fifo) on /tmp/ directory to communicate with CircuitRouter-Client (running on a different terminal ) where each client can send 'run INPUTFILE' commands and AdvShell will solve them and send the message "Circuit solved." back to the respective client. The client must wait for a command to be completed before sending the next one.

## ParSolver test script:	

To run the test script, run './doTest.sh NUMOFTHREADS INPUTFILE' inside CircuitRouter-ParSolver directory. It will run ParSolver on the input file multiple times with an increasing number of threads untill NUMOFTHREADS is reached. It writes the speedup ratio in a file with a .speedups.cvs extension.

	NUMOFTHREADS is any positive number (ideally the number of threads in the system)

	INPUTFILE is any .txt file under the 'inputs' folder

	EX: ./doTest.sh 8 ../inputs/random-x32-y32-z3-n96.txt
	It will write the speedups times in random-x32-y32-z3-n96.txt.speedups.cvs

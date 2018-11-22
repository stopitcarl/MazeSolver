i)


│ README.txt (this file)

│ ./compileAll

│

├───CircuitRouter-ParSolver 

│

├───CircuitRouter-SeqSolver 

│

├───inputs       

│

├───lib

│

└───results



ii)

COMPILATION:
	To compile both solutions at once run './compileAll.sh' in this directory.

	To compile each individually, run 'make' in both directories: CircuitRouter-ParSolver and CircuitRouter-SeqSolver.


EXECUTION:

	To run the executable, run './doTest.sh NUMOFTHREADS INPUTFILE' inside CircuitRouter-ParSolver.

	NUMOFTHREADS is any positive number (ideally the number of threads in the system)
.
	INPUTFILE is any .txt file under the 'inputs' folder

	EX: ./doTest.sh 8 ../inputs/random-x32-y32-z3-n96.txt




iii)

OS:

	Linux sigma02.ist.utl.pt 4.9.0-8-amd64 #1 SMP Debian 4.9.110-3+deb9u4 (2018-08-21) x86_64 GNU/Linux

Specs:

	Intel(R) Xeon(R) CPU E5-2620 0 @ 2.00GHz
	
	8 Threads ( virtually assigned from a cluster of 6 Core - 12 Thread CPUs)

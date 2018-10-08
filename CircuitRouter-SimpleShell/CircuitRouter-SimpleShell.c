/*
 * =============================================================================
 *
 * CircuitRouter-SimpleShell.c
 *
 * =============================================================================
 */

#define MAXINPUT 2048
#define MAXWORDSIZE 128
#define NUMOFWORDS 16

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "lib/commandlinereader.h"
#include "lib/vector.h"

struct threads {
	vector_t threadList;
	int lock;
};

struct threadState {
	int state;
	pid_t pid;
};

struct threads threadHistory; // TODO: static variable okay to use? better options?

struct test {
	pid_t pid;
	int signum;
	int status;
};

struct test horse;
void storeProcess(int pid, int status, int signum);


/* =============================================================================
 * handleChild : handles child (r/programmerhumor)
 * =============================================================================
 */
void handleChild(int signum) {
	int status = 0;
	int pid = wait(&status);
	// while ((int pid = waitpid(-1, &status, WNOHANG)) > 0); // TODO: way to get pid from waitpid?
	storeProcess(pid, status, signum);
}


/* =============================================================================
 * storeProcess : Stores process info for later consumption
 * =============================================================================
 */
void storeProcess(int pid, int status, int signum) {
	horse.signum = signum;
	horse.pid = pid;
	horse.status = status;
}

/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv) {
	pid_t pid = 0;
	int running = 1;

	int numArgs = 0;
	char cmdBuffer[MAXINPUT];
	char *argsRead[NUMOFWORDS];



	// Create thread history storage	
	// threadHistory.lock = 0;
	// threadHistory.threadList = vector_alloc(10); // TODO: Read MAXCHILDREN num from args 


	while (running) {
		printf(">");
		numArgs = readLineArguments(argsRead, NUMOFWORDS, cmdBuffer, MAXINPUT);
		if (numArgs == -1) {
			printf("readLineArguments encountered a problem\n");
			continue;
		}
		else {
			int i = 0;
			for (i = 0; i < numArgs; i++) {
				printf("%i: %s\n", i, argsRead[i]);
			}
		}

		pid = fork();
		if (pid < 0) {
			printf("Unable to fork\n");
			continue;
		}
		else if (pid == 0) {
			// child process
			sleep(4);
			printf("am i the child process\n");			
			exit(0);
		}
		else {
			// parent process
			signal(SIGCHLD, handleChild);
			printf("i am the parent process\n");
			sleep(2);
			printf("pid: %d,sigsum: %d, status: %d\n", horse.pid, horse.signum, horse.status);
			sleep(5);
			printf("pid: %d,sigsum: %d, status: %d\n", horse.pid, horse.signum, horse.status);
		}



	}

	exit(0);
}


/* =============================================================================
 *
 * End of CircuitRouter-SimpleShell.c
 *
 * =============================================================================
 */

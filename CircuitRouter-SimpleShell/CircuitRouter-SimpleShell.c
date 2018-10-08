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
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "lib/commandlinereader.h"
#include "lib/vector.h"

struct threads {
	vector_t * threadList;
	int lock;
};

struct threadState {
	int state;
	pid_t pid;
};

struct threads * threadHistory; // TODO: static variable okay to use? better options?



void storeProcess(int pid, int status);


/* =============================================================================
 * handleChild
 * =============================================================================
 */
void handleChild(int signum) {
	int status = 0;
	int pid = 0;
	// int pid = wait(&status);
	while ((pid = waitpid(-1, &status, WNOHANG)) <= 0); // TODO: way to get pid from waitpid?
	storeProcess(pid, status);
}


/* =============================================================================
 * storeProcess : Stores process info for later consumption
 * =============================================================================
 */
void storeProcess(int pid, int status) {
	struct threadState *  process = malloc(sizeof(struct threadState));
	process->state = status;
	process->pid = pid;
	assert(vector_pushBack(threadHistory->threadList, process) == TRUE);
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
	threadHistory = malloc(sizeof(struct threads));
	threadHistory->lock = 0;
	threadHistory->threadList = vector_alloc(10); // TODO: Read MAXCHILDREN num from args 


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

		if (strcmp(argsRead[0], "exit")) {
			break;
		}

		pid = fork();
		if (pid < 0) {
			printf("Unable to fork\n");
			continue;
		}
		else if (pid == 0) {
			// child process			
			printf("%s %s\n", argsRead[0], argsRead[1]);
			exit(0);
		}
		else {
			// parent process
			signal(SIGCHLD, handleChild);
			printf("i am the parent process\n");
		}
	}

	int i = vector_getSize(threadHistory->threadList);
	struct threadState *thread;
	for (; i > 0; ++i) {
		thread = (struct threadState*)vector_at(threadHistory->threadList, i);
		printf("pid: %d, state: %d\n", thread->pid, thread->state);
	}

	// TODO: free memory
	vector_free(threadHistory->threadList);
	free(threadHistory);

	exit(0);
}


/* =============================================================================
 *
 * End of CircuitRouter-SimpleShell.c
 *
 * =============================================================================
 */

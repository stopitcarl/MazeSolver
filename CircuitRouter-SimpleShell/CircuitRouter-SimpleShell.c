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
 * waitForThread: blocks untill thread is released (deprecated) TODO: erase this?
 * =============================================================================
 */
void waitForThread() {
	int status, pid;

	while ((pid = waitpid(-1, &status, WNOHANG)) <= 0);
	printf("process caught %d:%d\n", pid, status);
	storeProcess(pid, status);
	//--running_threads;
}

/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv) {
	pid_t pid = 0;
	int child_pid = 0;
	int status = 0;
	int running = TRUE;
	int available_threads = 2; // TODO: Get this for args
	int running_threads = 0;

	int numArgs = 0;
	char cmdBuffer[MAXINPUT];
	char *argsRead[NUMOFWORDS];

	// Create thread history storage	
	threadHistory = malloc(sizeof(struct threads));
	threadHistory->lock = 0;
	threadHistory->threadList = vector_alloc(10); // TODO: Read MAXCHILDREN num from args 


	do {
		// Catch a thread?
		if ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
			printf("process caught %d:%d\n", child_pid, status);
			storeProcess(child_pid, status);
			--running_threads;
		}

		printf(" >> ");
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

		// Exit 
		if (strcmp(argsRead[0], "exit") == 0) {
			running = FALSE; // TODO: redudant code?
			break;
		}
		else // Check if there are available threads
			if (running_threads == available_threads) {
				printf("All threads are busy, please try later\n"); // TODO: block untill a thread is released?
				continue;
			}
			else { ++running_threads; } // Update available threads

		pid = fork();
		if (pid < 0) {
			printf("Unable to fork\n");
			continue;
		}
		else if (pid == 0) {
			// child process						
			printf("child-> %s %s\n", argsRead[0], argsRead[1]); // TODO: launch SeqSolver
			sleep(2);
			exit(0);
		}
		else {
			// parent process
			//signal(SIGCHLD, handleChild); // TODO: choose between signal and waitpid
			printf("parent process\n");
		}

		// catch a child (pedo comment is pedo)
		do {
			if ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
				printf("process caught %d:%d\n", child_pid, status);
				storeProcess(child_pid, status);
				--running_threads;
			}
		} while (running_threads == 0);

	} while (running);

	while (running_threads > 0) {
		if ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
			printf("process caught %d:%d\n", child_pid, status);
			storeProcess(child_pid, status);
			--running_threads;
		}
	}

	int i = vector_getSize(threadHistory->threadList);
	printf("size is %d\n", i);
	struct threadState *thread;
	for (; i > 0; --i) {
		thread = (struct threadState*)vector_at(threadHistory->threadList, i - 1);
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

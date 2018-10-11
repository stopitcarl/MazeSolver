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
#include <inttypes.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "lib/commandlinereader.h"
#include "lib/vector.h"



struct taskState {
	int state;
	pid_t pid;
};

vector_t *taskHistory;
int running_tasks;


/* =============================================================================
 * storetask : Stores task info for later consumption
 * =============================================================================
 */
void storetask(int pid, int status) {
	struct taskState *task = malloc(sizeof(struct taskState));
	task->state = status;
	task->pid = pid;
	assert(vector_pushBack(taskHistory, task) == TRUE);
}

/* =============================================================================
 * waitForChild: Blocks untill child is released
 * =============================================================================
 */
void waitForChild() {
	int child_pid = 0, status = 0;
	while ((child_pid = waitpid(-1, &status, WNOHANG)) > -1) { // while there's tasks running
		if (child_pid > 0) {
			storetask(child_pid, status);
			--running_tasks;
			return;
		}
	}
}

/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char **argv) {
	pid_t pid = 0;
	int child_pid = 0;
	int status = 0;
	int running = TRUE;
	int available_tasks = 0; // TODO: Get this from args
	running_tasks = 0;

	int numArgs = 0;
	char cmdBuffer[MAXINPUT];
	char *argsRead[NUMOFWORDS];

	// Check if args were passed
	if (argc == 2 && (available_tasks = strtoumax(argv[1], NULL, 10))) {
		if (available_tasks == 0) {
			available_tasks = -1;
		}
		// printf("MAXCHILDREN are %d\n", available_tasks);
	}
	else {
		available_tasks = -1; // no args provided -> infinite tasks 
	}

	// Create task history storage	
	taskHistory = vector_alloc(10); // TODO: Read MAXCHILDREN num from args
	assert(taskHistory);

	// Shell loop
	do {
		// Read args from input
		printf(">>> ");
		numArgs = readLineArguments(argsRead, NUMOFWORDS, cmdBuffer, MAXINPUT);
		if (numArgs == -1) {
			printf("Couldn't read input.\n");
			continue;
		}
		else if (numArgs == 0) {
			continue;
		}

		// Catch a task
		if ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
			// printf("task caught %d:%d\n", child_pid, status);
			storetask(child_pid, status);
			--running_tasks;
		}

		// Exit
		if (strcmp(argsRead[0], "exit") == 0) {
			running = FALSE; // TODO: redudant code?  (may use later)
			break;
		}
		else if (strcmp(argsRead[0], "run") == 0) {

			if (running_tasks == available_tasks) {
				printf("All threads are busy. Waiting for a task to end\n");
				waitForChild(); // block untill any child task is finished							
				printf("Starting your task\n");
			}

			pid = fork();
			if (pid < 0) {
				printf("Unable to fork\n");
				--running_tasks;
				continue;
			}
			else if (pid == 0) { // child task
				if (!(numArgs == 2 && strcmp(argsRead[0], "run") == 0)) {
					exit(1);
				}
				char *args[] = { "./CircuitRouter-SeqSolver", argsRead[1], NULL };
				if (execv("../CircuitRouter-SeqSolver/./CircuitRouter-SeqSolver", args) == -1) {
					exit(1);
				}
			}
			else { // parent task
				++running_tasks;
			}
		}
		else { printf("Unknown command.\n"); }
	} while (running);


	// Catch all remaining running tasks
	while (running_tasks > 0) {
		if ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
		{
			storetask(child_pid, status);
			--running_tasks;
		}
	}

	int i;
	int j = vector_getSize(taskHistory);
	struct taskState *task;
	for (i = 0; i < j; ++i) {
		task = (struct taskState *)vector_at(taskHistory, i);
		printf("CHILD EXITED (PID=%d; return %s)\n", task->pid, task->state == 0 ? "OK" : "NOK");
		free(task);
	}
	puts("End");

	// Free remaining memory
	vector_free(taskHistory);

	exit(0);
}

/* =============================================================================
 *
 * End of CircuitRouter-SimpleShell.c
 *
 * =============================================================================
 */

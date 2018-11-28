/*
 * =============================================================================
 *
 * CircuitRouter-AdvShell.c
 *
 * =============================================================================
 */

#define MAXINPUT 2048
#define MAXWORDSIZE 128
#define NUMOFWORDS 16

#include <assert.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "lib/commandlinereader.h"
#include "lib/timer.h"
#include "lib/types.h"
#include "lib/vector.h"



struct taskState {
	int state;
	pid_t pid;
	TIMER_T startTime, stopTime;
};

vector_t *taskHistory;
int running_tasks;

/* =============================================================================
 * create : Stores task info for later consumption
 * =============================================================================
 */
void addChild(vector_t * tasks, pid_t pid) {
	struct taskState *task = malloc(sizeof(struct taskState));
	TIMER_READ(task->startTime);
	task->pid = pid;
	assert(vector_pushBack(taskHistory, task) == TRUE);
	++running_tasks;
}


/* =============================================================================
 * storetask : Stores task info for later consumption
 * =============================================================================
 */
void storeChild(pid_t pid, int status) {
	struct taskState *task = NULL;
	long i = vector_getSize(taskHistory) - 1;

	for (; i >= 0; i--) {
		task = vector_at(taskHistory, i);
		if (task->pid == pid)
			break;
	}
	assert(task); // Check that a task was found with given pid	

	task->state = status;
	TIMER_READ(task->stopTime);
	--running_tasks;
}

/* =============================================================================
 * waitForChild: Blocks untill child is released
 * =============================================================================
 */
void waitForChild() {
	int child_pid = 0, status = 0;
	while ((child_pid = waitpid(-1, &status, WNOHANG)) > -1) { // while there's tasks running
		if (child_pid > 0) {
			storeChild(child_pid, status);
			return;
		}
	}
}

/* =============================================================================
 * childReaper: Handles SIGCHLD
 * =============================================================================
 */
void childReaper(int sig)
{
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
		storeChild(pid, status);

}



/* =============================================================================
 * printTaskInfo: Blocks untill child is released
 * =============================================================================
 */
void printTaskInfo(struct taskState * task) {
	printf("CHILD EXITED (PID=%d; return ", task->pid);

	// Print OK if process exited normally, NOK otherwise
	if (WIFEXITED(task->state) && WEXITSTATUS(task->state) == 0)
		printf("OK;");
	else
		printf("NOK;");
	printf(" %.0f s)\n", TIMER_DIFF_SECONDS(task->startTime, task->stopTime)); // TODO: check if output has decimals correct
}

/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char **argv) {
	pid_t pid = 0;
	int running = TRUE;
	int available_tasks = 0;
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
		available_tasks = -1; // no args provided = infinite tasks
	}

	// Set childReaper as a handler for SIGCHLD
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = childReaper;
	sa.sa_flags = SA_RESTART;
	assert(sigaction(SIGCHLD, &sa, NULL) == 0);


	// Create task history storage	
	taskHistory = vector_alloc(available_tasks > 0 ? available_tasks * 2 : 8);
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

		// Exit
		if (strcmp(argsRead[0], "exit") == 0) {
			running = FALSE; // redudant code?  (may use later)
			break;
		}
		else if (strcmp(argsRead[0], "run") == 0) {
			if (numArgs < 2) { printf("No input file specified\n"); continue; }
			if (running_tasks == available_tasks) {
				printf("All threads are busy. Waiting for a task to end\n");
				waitForChild(); // block untill any child task is finished							
				printf("Starting your task\n");
			}

			pid = fork();
			if (pid < 0) {
				printf("Unable to fork\n");
				continue;
			}
			else if (pid == 0) { // child task
				if (!(numArgs == 2 && strcmp(argsRead[0], "run") == 0))
					exit(1);
				char *args[] = { "CircuitRouter-SeqSolver", argsRead[1], NULL };
				if (execv("../CircuitRouter-SeqSolver/CircuitRouter-SeqSolver", args) == -1)
					exit(1);
				else
					exit(0);
			}
			else { // parent task
				addChild(taskHistory, pid);
			}
		}
		else { printf("Unknown command.\n"); }
	} while (running);


	// Wait untill all remaining children are reaped
	while (running_tasks > 0);

	int i;
	int j = vector_getSize(taskHistory);
	struct taskState *task;
	for (i = 0; i < j; ++i) {
		task = (struct taskState *)vector_at(taskHistory, i);
		printTaskInfo(task);
		free(task);
	}

	// Free remaining memory
	vector_free(taskHistory);

	exit(0);
}

/* =============================================================================
 *
 * End of CircuitRouter-AdvShell.c
 *
 * =============================================================================
 */

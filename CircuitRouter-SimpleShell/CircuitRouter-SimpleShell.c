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

struct threads
{
	vector_t *threadList;
};

struct threadState
{
	int state;
	pid_t pid;
};

struct threads *threadHistory; // TODO: static variable okay to use? better options?

void storeProcess(int pid, int status);

/* =============================================================================
 * storeProcess : Stores process info for later consumption
 * =============================================================================
 */
void storeProcess(int pid, int status)
{
	struct threadState *process = malloc(sizeof(struct threadState));
	process->state = status;
	process->pid = pid;
	assert(vector_pushBack(threadHistory->threadList, process) == TRUE);
}

/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char **argv)
{
	pid_t pid = 0;
	int child_pid = 0;
	int status = 0;
	int running = TRUE;
	int available_threads = 0; // TODO: Get this from args
	int running_threads = 0;

	int numArgs = 0;
	char cmdBuffer[MAXINPUT];
	char *argsRead[NUMOFWORDS];

	// Check if args were passed
	if (argc == 2 && (available_threads = strtoumax(argv[1], NULL, 10)))
	{
		printf("MAXTHREADS are %d\n", available_threads);
	}
	else
	{
		available_threads = -1;
	}

	// Create thread history storage
	threadHistory = malloc(sizeof(struct threads));
	threadHistory->threadList = vector_alloc(10); // TODO: Read MAXCHILDREN num from args

	do
	{

		// Read args from input
		printf(" >> ");
		numArgs = readLineArguments(argsRead, NUMOFWORDS, cmdBuffer, MAXINPUT);
		if (numArgs == -1)
		{
			printf("readLineArguments encountered a problem\n");
			continue;
		}
		else if (numArgs == 0)
		{
			continue;
		}

		// Catch a thread?
		if ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
		{
			printf("process caught %d:%d\n", child_pid, status);
			storeProcess(child_pid, status);
			--running_threads;
		}

		// Exit
		if (strcmp(argsRead[0], "exit") == 0)
		{
			running = FALSE; // TODO: redudant code?
			break;
		}
		else // Check if there are available threads
			if (running_threads == available_threads)
		{
			printf("All threads are busy, please try later\n"); // TODO: block untill a thread is released?
			continue;
		}

		pid = fork();
		if (pid < 0)
		{
			printf("Unable to fork\n");
			continue;
		}
		else if (pid == 0) // child process
		{

			if (!(numArgs == 2 && strcmp(argsRead[0], "run") == 0))
			{
			}
			char *args[] = {"./CircuitRouter-SeqSolver", argsRead[1], NULL};

			if (execv("../CircuitRouter-SeqSolver/", args) == -1)
			{
				exit(1);
			}
			int i = 0;
			for (i = 0; i < numArgs; i++)
			{
				//printf("%i: %s\n", i, argsRead[i]);
			}
			exit(0);
		}
		else
		{
			// parent process
			++running_threads;
			//signal(SIGCHLD, handleChild); // TODO: choose between signal and waitpid
		}
	} while (running);

	// Catch all running threads
	while (running_threads > 0)
	{
		if ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
		{
			printf("process caught %d:%d\n", child_pid, status);
			storeProcess(child_pid, status);
			--running_threads;
		}
	}

	int i = vector_getSize(threadHistory->threadList);
	struct threadState *thread;
	for (; i > 0; --i)
	{
		thread = (struct threadState *)vector_at(threadHistory->threadList, i - 1);
		printf("CHILD EXITED (PID=%d; return %s\n", thread->pid, thread->state == 0 ? "OK" : "NOK");
		free(thread);
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

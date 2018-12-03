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
#define STDIN 0

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "lib/commandlinereader.h"
#include "lib/timer.h"
#include "lib/types.h"
#include "lib/vector.h"


typedef struct client {
	char * name;
	int alive;
	int fd;
} Client;

typedef struct taskState {
	int state;
	pid_t pid;
	time_t startTime, stopTime;
	Client* client;
} Task;

vector_t *taskHistory;
vector_t *clientList;
volatile int running_tasks;
int active_clients;


/* =============================================================================
 * registertClient
 * =============================================================================
 */
void registertClient(char * clientName, char * clientPipe) {

	// Create client
	Client *clientPtr = malloc(sizeof(Client));
	clientPtr->name = malloc(sizeof(char) * (strlen(clientName) + 1));
	strcpy(clientPtr->name, clientName);
	clientPtr->alive = TRUE;

	// Connect to his pipe
	assert((clientPtr->fd = open(clientPipe, O_WRONLY | O_NONBLOCK)) > 0);

	// Add client to list
	assert(vector_pushBack(clientList, clientPtr) == TRUE);
	active_clients++;
}

/* =============================================================================
 * answerClient
 * =============================================================================
 */
void answerClient(Client * clientPtr, char * answer) {
	assert(clientPtr != NULL);
	if (clientPtr->alive == TRUE)
		assert(write(clientPtr->fd, answer, strlen(answer)) > 0);
}

/* =============================================================================
 * getClient
 * =============================================================================
 */
Client * getClient(char * name) {
	assert(name != NULL);
	int i = 0;
	Client* clientPtr;
	for (; i < vector_getSize(clientList); i++) {
		clientPtr = vector_at(clientList, i);
		if (strcmp(name, clientPtr->name) == 0)
			return clientPtr;
	}
	return NULL;
}

/* =============================================================================
 * disconnectClient
 * =============================================================================
 */
void disconnectClient(char * clientName) {
	int i = 0;
	Client client;
	for (; i < vector_getSize(clientList); i++) {
		client = *(Client*)vector_at(clientList, i);
		if (strcmp(clientName, client.name) == 0) {
			client.alive = FALSE;
			break;
		}
	}
	active_clients--;
}

/* =============================================================================
 * tasksRunning: Add and fetch from
 * =============================================================================
 */
int tasksRunning(int add) {
	sigset_t x;
	sigemptyset(&x);
	sigaddset(&x, SIGCHLD);
	sigprocmask(SIG_BLOCK, &x, NULL);
	running_tasks += add;
	int result = running_tasks;
	sigprocmask(SIG_UNBLOCK, &x, NULL);
	return result;
}

/* =============================================================================
 * storetask : Stores task info for later consumption
 * =============================================================================
 */
void storeChild(pid_t pid, int status) {
	Task *task = NULL;
	long i = vector_getSize(taskHistory) - 1;

	for (; i >= 0; i--) { // Process is most likely at the top of the list
		task = vector_at(taskHistory, i);
		if (task->pid == pid)
			break;
	}
	assert(task); // Check that a task was found with given pid	
	if (task->client != NULL) {
		answerClient(task->client, "Circuit solved\n");
	}
	task->state = status;
	time(&(task->stopTime));
	tasksRunning(-1);
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
 * addChild : Add child process info to list
 * =============================================================================
 */
void addChild(vector_t * tasks, pid_t pid, Client *clientPtr) {
	Task *task = malloc(sizeof(Task));
	time(&(task->startTime));
	task->pid = pid;
	task->client = clientPtr;
	assert(vector_pushBack(taskHistory, task) == TRUE);
	tasksRunning(1);
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
void printTaskInfo(Task * task) {
	printf("CHILD EXITED (PID=%d; return ", task->pid);

	// Print OK if process exited normally, NOK otherwise
	if (WIFEXITED(task->state) && WEXITSTATUS(task->state) == 0)
		printf("OK;");
	else
		printf("NOK;");
	printf(" %ld s)\n", task->stopTime - task->startTime);
}

/* =============================================================================
 * makePipeName
 * =============================================================================
 */
char * makePipeName(char * pathName) {
	char *str = malloc(sizeof(char) *
		(strlen(pathName) + strlen("/tmp/.pipe") + 1)); // implementation indepedant
	strcpy(str, "/tmp/");
	strcat(str, basename(pathName));
	strcat(str, ".pipe");
	return str;
}

/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char **argv) {
	// Shell vars
	pid_t pid = 0;
	int running = TRUE;
	int available_tasks = 0;
	running_tasks = 0;
	int numArgs = 0;
	char cmdBuffer[MAXINPUT];
	char *argsRead[NUMOFWORDS];
	// Client vars
	char * pipe_in_name = makePipeName(argv[0]);
	int fd_in;
	int isClient = 0;
	active_clients = 0;

	// Check if args were passed
	if (argc == 2 && (available_tasks = strtoumax(argv[1], NULL, 10))) {
		if (available_tasks == 0)
			available_tasks = -1;
	}
	else {
		available_tasks = -1; // no args provided = infinite tasks
	}


	// Create input pipe
	fd_set fds;
	unlink(pipe_in_name);
	assert(mkfifo(pipe_in_name, 0777) == 0);
	assert((fd_in = open(pipe_in_name, O_RDWR | O_NONBLOCK)) > 0);
	int maxfd = (fd_in > STDIN) ? fd_in : STDIN;

	// Set childReaper as a handler for SIGCHLD
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = childReaper;
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	assert(sigaction(SIGCHLD, &sa, NULL) == 0);


	// Create task history storage	
	taskHistory = vector_alloc(available_tasks > 0 ? available_tasks * 2 : 8);
	assert(taskHistory);

	// Create client list
	clientList = vector_alloc(4);
	assert(clientList);


	// Shell loop
	do {

		printf(">>> ");
		fflush(stdout);
		isClient = 0;

		// Select inputs to listen to
		FD_ZERO(&fds);
		FD_SET(STDIN, &fds);
		FD_SET(fd_in, &fds);
		// Block untill anyone of the input is available for read
		while (select(maxfd + 1, &fds, NULL, NULL, NULL) == -1 && errno == EINTR);

		// Read args from input		
		if (FD_ISSET(STDIN, &fds)) {
			numArgs = readLineArguments(argsRead, NUMOFWORDS, cmdBuffer, MAXINPUT);
		}
		else if (FD_ISSET(fd_in, &fds)) {
			isClient = 1;
			numArgs = readLineArgumentsFd(fd_in, argsRead, NUMOFWORDS, cmdBuffer, MAXINPUT);
		}
		else {
			printf("Oops, an error has occured while reading input\n");
			continue;
		}

		if (numArgs == -1) {
			printf("Couldn't read input.\n");
			continue;
		}
		else if (numArgs == 0) {
			continue;
		}

		// Handle client interactions
		if (isClient) {

			// Print client message
			printf("\nReceived: ");
			for (int i = 0; i < numArgs; i++)
				printf(" %s", argsRead[i]);
			printf("\n");
			fflush(stdout);

			// Connect
			if (strcmp(argsRead[0], "c") == 0) {
				registertClient(argsRead[1], argsRead[2]);
				getClient(argsRead[1]);
				continue;
			}

			Client * clientPtr = getClient(argsRead[1]);
			assert(clientPtr);
			// Disconnect
			if (strcmp(argsRead[0], "e") == 0) {
				disconnectClient(argsRead[1]);
				continue;;
			}
			// Run
			else if (strcmp(argsRead[0], "r") == 0 && numArgs == 4) {
				if (strcmp(argsRead[2], "run") == 0) {
					if (tasksRunning(0) == available_tasks) {
						waitForChild(); // block untill any child task is finished													
					}

					pid = fork();
					if (pid < 0) {
						continue;
					}
					else if (pid == 0) { // child task
						if (numArgs != 4)
							exit(-1);
						char *args[] = { "CircuitRouter-SeqSolver", argsRead[3], NULL };
						if (execv("../CircuitRouter-SeqSolver/CircuitRouter-SeqSolver", args) == -1)
							exit(-1);
						else
							exit(0);
					}
					else { // parent task
						addChild(taskHistory, pid, clientPtr);
					}
				}
				else {
					answerClient(clientPtr, "Command not supported.\n");
				}
			}
			else {
				answerClient(clientPtr, "Command not supported.\n");
			}
		}
		// Handle stdin interactions
		else {
			// Exit
			if (strcmp(argsRead[0], "exit") == 0) {
				running = FALSE; // redudant code?  (may use later)
				break;
			}
			else if (strcmp(argsRead[0], "run") == 0) {
				if (numArgs < 2) { printf("No input file specified\n"); continue; }
				if (tasksRunning(0) == available_tasks) {
					waitForChild(); // block untill any child task is finished												
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
					addChild(taskHistory, pid, NULL);
				}
			}
			else { printf("Unknown command.\n"); }
		}
	} while (running);


	// Wait untill all remaining children are reaped
	while (tasksRunning(0) > 0)
		pause();

	// Print children process history
	int i;
	int j = vector_getSize(taskHistory);
	Task *task;
	for (i = 0; i < j; ++i) {
		task = (Task *)vector_at(taskHistory, i);
		printTaskInfo(task);
		free(task);
	}

	// Clear client list
	j = vector_getSize(clientList);
	Client *clientPtr;
	for (i = 0; i < j; ++i) {
		clientPtr = (Client *)vector_at(clientList, i);
		free(clientPtr->name);
		free(clientPtr);
	}

	// Free remaining memory
	vector_free(taskHistory);
	vector_free(clientList);
	unlink(pipe_in_name);
	free(pipe_in_name);

	exit(0);
}

/* =============================================================================
 *
 * End of CircuitRouter-AdvShell.c
 *
 * =============================================================================
 */

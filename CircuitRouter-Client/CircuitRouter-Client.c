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
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lib/commandlinereader.h"
#include "lib/vector.h"


 /* =============================================================================
  * connect
  * =============================================================================
  */
void connect(char* pipe, int *fd) {
	printf("Connect to pipe %s\n", pipe);
	unlink(pipe);
	assert(mkfifo(pipe, 0777) == 0);
	*fd = open(pipe, O_WRONLY);
}

/* =============================================================================
 * exit
 * =============================================================================
 */
void disconnect(char* pipe) {
	printf("Send disconnection request\n");
	// TODO: discconection request
	sleep(1);
	unlink(pipe);
	free(pipe);
	printf("Exited\n");
}

char * makePipeName(int pid) {
	int length = snprintf(NULL, 0, "%d", pid);
	char *str = malloc(sizeof(char) * (length + 11));
	sprintf(str, "/tmp/%d.pipe", pid);
	return str;
}



/* =============================================================================
 * main
 * =============================================================================
 */
int main() {
	int numArgs = 0;
	int fd;
	char cmdBuffer[MAXINPUT];
	char *argsRead[NUMOFWORDS];

	char * fifo = makePipeName(getpid());
	connect(fifo, &fd);

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
		if (strcmp(argsRead[0], "exit") == 0) { break; }
		// Run
		else if (strcmp(argsRead[0], "run") == 0) {
			write(fd, "Asking server to run\n", 19);
		}
		else { printf("Unknown command.\n"); }
	} while (1);

	disconnect(fifo);

	return 0;
}

/* =============================================================================
 *
 * End of CircuitRouter-SimpleShell.c
 *
 * =============================================================================
 */

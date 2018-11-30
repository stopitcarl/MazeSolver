/*
 * =============================================================================
 *
 * CircuitRouter-Client.c
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
#include <errno.h>
#include "lib/commandlinereader.h"
#include "lib/vector.h"


int fd_in, fd_out;
char process_name[64];
char exit_request[64];
char * pipe_in_name;

/* =============================================================================
 * connect
 * =============================================================================
 */
void connect(char* pipe_name) {
	printf("Connect to pipe %s\n", pipe_name); // TODO: Clean this
	// Initialize request strings
	sprintf(process_name, "%d", getpid());
	sprintf(exit_request, "e %s\n", process_name);
	// Create pipe
	unlink(pipe_name);
	assert(mkfifo(pipe_name, 0777) == 0);
	assert((fd_in = open(pipe_in_name, O_RDWR | O_NONBLOCK)) > 0);
	printf("pipe created\n");
	// Send syn request
	char buf[512];
	int size = sprintf(buf, "c %s %s\n", process_name, pipe_name);
	write(fd_out, buf, size);
	printf("wrote\n");
}

/* =============================================================================
 * writeCommand
 * =============================================================================
 */
void writeCommand(char *word[NUMOFWORDS], int numWords) {
	int i = 0, j;
	char buf[512];
	// Create command type and process id
	if (numWords == 0) return;
	j = sprintf(buf, "r %s", process_name);
	printf("init j = %d\n", j);
	// Add command args
	for (i = 0; i < numWords; i++) {
		sprintf(&buf[j], " %s", word[i]);
		j += strlen(word[i]) + 1;
		printf("loop j = %d\n", j);
	}
	printf("j = %d\n", j);
	j += sprintf(&buf[j], "\n");
	printf("j = %d\n", j);
	// Write command	
	write(fd_out, buf, j);
}


/* =============================================================================
 * waitAnswer
 * =============================================================================
 */
void waitAnswer() {
	char buf_in[512];

	// TODO: clean this
	// Block untill message is read to buffer
	int size = 0;
	while ((size = read(fd_in, buf_in, 512)) < 0);
	assert(size < 512);
	buf_in[size] = '\0';
	printf("Read :%s: %d bytes from pipe\n", buf_in, size);
}




/* =============================================================================
 * disconnect
 * =============================================================================
 */
void disconnect() {
	printf("Sending disconnection request\n");
	// TODO: discconection request
	write(fd_out, exit_request, strlen(exit_request) + 1);
	assert(close(fd_out) == 0);
	assert(close(fd_in) == 0);
	unlink(pipe_in_name);
	free(pipe_in_name);
	printf("Exiting...\n");
	exit(0);
}

/* =============================================================================
 * makePipeName
 * =============================================================================
 */
char * makePipeName(int pid) {
	int length = snprintf(NULL, 0, "%d", pid); // get number length
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
	char cmdBuffer[MAXINPUT];
	char *argsRead[NUMOFWORDS];
	char * pipe_out_name = "/tmp/CircuitRouter-AdvShell.pipe";
	pipe_in_name = makePipeName(getpid());

	// Set disconnect as a handler for SIGINT
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = disconnect;
	sa.sa_flags = SA_RESTART;
	assert(sigaction(SIGINT, &sa, NULL) == 0);

	// Connect to server pipe
	while ((fd_out = open(pipe_out_name, O_WRONLY)) == -1 && errno == EINTR);
	assert(fd_out > 0);

	// Create client pipe and tell server pipe name
	connect(pipe_in_name);
	printf("Connected\n");
	// Shell loop
	do {
		// Read args from input
		printf(">>> ");
		fflush(stdout);
		numArgs = readLineArgumentsFd(0, argsRead, NUMOFWORDS, cmdBuffer, MAXINPUT);
		if (numArgs == -1) {
			printf("Couldn't read input.\n");
			continue;
		}
		else if (numArgs == -2) { // IF EOF is reached
			break;
		}
		else if (numArgs == 0) {
			continue;
		}
		printf("Writing command\n");
		writeCommand(argsRead, numArgs);
		printf("Awaiting answer\n");
		waitAnswer();

	} while (1);

	disconnect();

	return 0;
}

/* =============================================================================
 *
 * End of CircuitRouter-Client.c
 *
 * =============================================================================
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * This code is an adaptation of the Lee algorithm's implementation originally included in the STAMP Benchmark
 * by Stanford University.
 *
 * The original copyright notice is included below.
 *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * Unless otherwise noted, the following license applies to STAMP files:
 *
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 *
 * CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */


#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>
#include <stdlib.h>
#include "lib/list.h"
#include "maze.h"
#include "router.h"
#include "lib/timer.h"
#include "lib/types.h"

enum param_types {
	PARAM_BENDCOST = (unsigned char)'b',
	PARAM_XCOST = (unsigned char)'x',
	PARAM_YCOST = (unsigned char)'y',
	PARAM_ZCOST = (unsigned char)'z',
};

enum param_defaults {
	PARAM_DEFAULT_BENDCOST = 1,
	PARAM_DEFAULT_XCOST = 1,
	PARAM_DEFAULT_YCOST = 1,
	PARAM_DEFAULT_ZCOST = 2,
};

bool_t global_doPrint = TRUE;
char* global_inputFile = NULL;
long global_params[256]; /* 256 = ascii limit */


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage(const char* appName) {
	printf("Usage: %s [options]\n", appName);
	puts("\nOptions:                            (defaults)\n");
	printf("    b <INT>    [b]end cost          (%i)\n", PARAM_DEFAULT_BENDCOST);
	printf("    p          [p]rint routed maze  (false)\n");
	printf("    x <UINT>   [x] movement cost    (%i)\n", PARAM_DEFAULT_XCOST);
	printf("    y <UINT>   [y] movement cost    (%i)\n", PARAM_DEFAULT_YCOST);
	printf("    z <UINT>   [z] movement cost    (%i)\n", PARAM_DEFAULT_ZCOST);
	printf("    h          [h]elp message       (false)\n");
	exit(1);
}


/* =============================================================================
 * setDefaultParams
 * =============================================================================
 */
static void setDefaultParams() {
	global_params[PARAM_BENDCOST] = PARAM_DEFAULT_BENDCOST;
	global_params[PARAM_XCOST] = PARAM_DEFAULT_XCOST;
	global_params[PARAM_YCOST] = PARAM_DEFAULT_YCOST;
	global_params[PARAM_ZCOST] = PARAM_DEFAULT_ZCOST;
	global_doPrint = TRUE;
}

/* =============================================================================
 * createOutputFile
 * =============================================================================
 */
void createOutputFile(const char*fname) {
	const char ext[] = ".res";
	const char extOld[] = ".old";
	char fullName[strlen(fname) + strlen(ext) + 1];
	char fullNameOld[strlen(fname) + strlen(ext) + strlen(extOld) + 1];

	// Create filename (add .res)
	strcpy(fullName, fname);
	strcat(fullName, ext);

	// if file exists: rename (add .old)
	if (access(fullName, W_OK) != -1) {
		strcpy(fullNameOld, fullName);
		strcat(fullNameOld, extOld);
		assert(rename(fullName, fullNameOld) == 0);
	}


	// Create result file and redirect stdout there
	assert(freopen(fullName, "w", stdout) != NULL);
}



/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static FILE * parseArgs(long argc, char* const argv[]) {
	long opt;
	FILE * fileToRead;

	opterr = 0;

	setDefaultParams();

	while ((opt = getopt(argc, argv, "hb:x:y:z:")) != -1) {
		switch (opt) {
		case 'b':
		case 'x':
		case 'y':
		case 'z':
			global_params[(unsigned char)opt] = atol(optarg);
			break;
		case '?':
		case 'h':
		default:
			opterr++;
			break;
		}
	}

	if (opterr) {
		displayUsage(argv[0]);
	}

	// If opt doesnt match options, assume it's the file name			
	fileToRead = fopen(argv[optind], "r");
	assert(fileToRead);
	createOutputFile(argv[optind]);

	return fileToRead;
}


/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv) {

	int stdout_copy = dup(1);

	// Redirect error messages			
	assert(freopen("stderr.log", "a", stderr) != NULL); // dont erase previous stderr info (for better debugging)



	// Open file
	FILE * file = parseArgs(argc, (char** const)argv);
	maze_t* mazePtr = maze_alloc();
	assert(mazePtr);
	// Read maze from file
	long numPathToRoute = maze_read(mazePtr, file);
	router_t* routerPtr = router_alloc(global_params[PARAM_XCOST],
		global_params[PARAM_YCOST],
		global_params[PARAM_ZCOST],
		global_params[PARAM_BENDCOST]);
	assert(routerPtr);
	list_t* pathVectorListPtr = list_alloc(NULL);
	assert(pathVectorListPtr);

	// Solve maze
	router_solve_arg_t routerArg = { routerPtr, mazePtr, pathVectorListPtr };
	TIMER_T startTime;
	TIMER_READ(startTime);

	router_solve((void *)&routerArg);

	TIMER_T stopTime;
	TIMER_READ(stopTime);

	long numPathRouted = 0;
	list_iter_t it;
	list_iter_reset(&it, pathVectorListPtr);
	while (list_iter_hasNext(&it, pathVectorListPtr)) {
		vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
		numPathRouted += vector_getSize(pathVectorPtr);
	}
	printf("Paths routed    = %li\n", numPathRouted);
	printf("Elapsed time    = %f seconds\n", TIMER_DIFF_SECONDS(startTime, stopTime));



	// Check solution and clean up
	assert(numPathRouted <= numPathToRoute);
	bool_t status = maze_checkPaths(mazePtr, pathVectorListPtr, global_doPrint);
	assert(status == TRUE);
	puts("Verification passed.");

	maze_free(mazePtr);
	router_free(routerPtr);

	list_iter_reset(&it, pathVectorListPtr);
	while (list_iter_hasNext(&it, pathVectorListPtr)) {
		vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
		vector_t* v;
		while ((v = vector_popBack(pathVectorPtr))) {
			// v stores pointers to longs stored elsewhere; no need to free them here
			vector_free(v);
		}
		vector_free(pathVectorPtr);
	}
	list_free(pathVectorListPtr);

	// Reset output to sdtout
	fflush(stdout);
	dup2(stdout_copy, 1);

	exit(0);
}

/* =============================================================================
 *
 * End of CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */

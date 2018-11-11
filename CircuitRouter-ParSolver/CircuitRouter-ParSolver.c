#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> /* Definition of AT_* constants */
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "lib/list.h"
#include "maze.h"
#include "router.h"
#include "lib/timer.h"
#include "lib/types.h"
#include "lib/mutex.h"

enum param_types
{
	PARAM_BENDCOST = (unsigned char)'b',
	PARAM_XCOST = (unsigned char)'x',
	PARAM_YCOST = (unsigned char)'y',
	PARAM_ZCOST = (unsigned char)'z',
};

enum param_defaults
{
	PARAM_DEFAULT_BENDCOST = 1,
	PARAM_DEFAULT_XCOST = 1,
	PARAM_DEFAULT_YCOST = 1,
	PARAM_DEFAULT_ZCOST = 2,
};

bool_t global_doPrint = TRUE;
unsigned long MAX_THREADS = 0;
char *global_inputFile = NULL;
long global_params[256]; /* 256 = ascii limit */

/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage(const char *appName)
{
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
static void setDefaultParams()
{
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
void createOutputFile(const char *fname)
{
	const char ext[] = ".res";
	const char extOld[] = ".old";
	char fullName[strlen(fname) + strlen(ext) + 1];
	char fullNameOld[strlen(fname) + strlen(ext) + strlen(extOld) + 1];

	// Create filename (add .res)
	strcpy(fullName, fname);
	strcat(fullName, ext);

	// if file exists: rename (add .old)
	if (access(fullName, W_OK) != -1)
	{
		strcpy(fullNameOld, fullName);
		strcat(fullNameOld, extOld);
		assert(rename(fullName, fullNameOld) == 0);
	}

	// Create result file and redirect stdout there
	freopen(fullName, "w", stdout);
}

/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static FILE *parseArgs(long argc, char *const argv[])
{
	long opt;
	FILE *fileToRead;
	opterr = 0;

	setDefaultParams();

	while ((opt = getopt(argc, argv, "hb:x:y:z:t:")) != -1)
	{
		switch (opt)
		{
		case 't':
			MAX_THREADS = strtol(optarg, NULL, 10);
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

	if (opterr)
	{
		displayUsage(argv[0]);
	}

	assert(MAX_THREADS != 0);
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
int main(int argc, char **argv)
{

	// Redirect error messages
	//freopen("stderr.log", "a", stderr); // dont erase previous stderr info (for better debugging)

	// Open file
	FILE *file = parseArgs(argc, (char **const)argv);
	maze_t *mazePtr = maze_alloc();
	assert(mazePtr);

	// Read maze from file
	long numPathToRoute = maze_read(mazePtr, file);
	router_t *routerPtr = router_alloc(global_params[PARAM_XCOST],
		global_params[PARAM_YCOST],
		global_params[PARAM_ZCOST],
		global_params[PARAM_BENDCOST]);
	assert(routerPtr);
	list_t *pathVectorListPtr = list_alloc(NULL);
	assert(pathVectorListPtr);

	// Solve maze
	router_solve_arg_t routerArg = { routerPtr, mazePtr, pathVectorListPtr };
	TIMER_T startTime;
	TIMER_READ(startTime);

	// Initialize mutexes
	queue_mutex_init();
	path_mutex_init();
	grid_mutex_init(mazePtr->gridPtr);
	pthread_t thread_id[MAX_THREADS];

	int i = 0;
	for (; i < MAX_THREADS; i++)
		pthread_create(&thread_id[i], NULL, router_solve, (void *)&routerArg);

	for (i = 0; i < MAX_THREADS; i++)
		pthread_join(thread_id[i], NULL);

	TIMER_T stopTime;
	TIMER_READ(stopTime);


	long numPathRouted = 0;
	list_iter_t it;
	list_iter_reset(&it, pathVectorListPtr);
	while (list_iter_hasNext(&it, pathVectorListPtr))
	{
		vector_t *pathVectorPtr = (vector_t *)list_iter_next(&it, pathVectorListPtr);
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
	grid_mutex_free();

	list_iter_reset(&it, pathVectorListPtr);
	while (list_iter_hasNext(&it, pathVectorListPtr))
	{
		vector_t *pathVectorPtr = (vector_t *)list_iter_next(&it, pathVectorListPtr);
		vector_t *v;
		while ((v = vector_popBack(pathVectorPtr)))
		{
			// v stores pointers to longs stored elsewhere; no need to free them here
			vector_free(v);
		}
		vector_free(pathVectorPtr);
	}
	list_free(pathVectorListPtr);

	exit(0);
}

/* =============================================================================
 *
 * End of CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */

#include <stdio.h>
#include "mutex.h"

static pthread_mutex_t queue_mutex;
static pthread_mutex_t path_mutex;
static pthread_mutex_t *grid_mutex;
static long* pointsOfGrid;
static size_t sizeOfPoints;


void queue_mutex_init()
{
	assert(pthread_mutex_init(&queue_mutex, NULL) == 0);
}

void queue_mutex_lock()
{
	assert(pthread_mutex_lock(&queue_mutex) == 0);
}

void queue_mutex_unlock()
{
	assert(pthread_mutex_unlock(&queue_mutex) == 0);
}

void path_mutex_init() {
	assert(pthread_mutex_init(&path_mutex, NULL) == 0);
}

void path_mutex_lock() {
	assert(pthread_mutex_lock(&path_mutex) == 0);
}

void path_mutex_unlock() {
	assert(pthread_mutex_unlock(&path_mutex) == 0);
}

void grid_mutex_init(grid_t* grid)
{
	int i;
	pointsOfGrid = grid->points;
	sizeOfPoints = grid->width * grid->height* grid->depth;

	// Create and initialize array of mutexes
	grid_mutex = malloc(sizeof(pthread_mutex_t) * sizeOfPoints);
	for (i = 0; i < sizeOfPoints; i++)
		assert(pthread_mutex_init(&grid_mutex[i], NULL) == 0);
}

int grid_mutex_lock(vector_t *points)
{
	size_t i = 0;
	short tries = 0;
	size_t size = vector_getSize(points);
	long * point;

	for (; i < size; i++) {
		point = (long*)vector_at(points, i);
		while (pthread_mutex_trylock(&grid_mutex[point - pointsOfGrid]) != 0) {
			if (tries++ < NUMBEROFTRIES)
				nanosleep(SLEEPTIME);
			else {
				grid_mutex_unlock_partial(points, i); // don't leave locked points
				return -1;
			}
		}
	}
	return 0;
}

void grid_mutex_unlock(vector_t *points)
{
	size_t i = 0;
	size_t size = vector_getSize(points);
	for (; i < size; i++)
		assert(pthread_mutex_unlock(&grid_mutex[(long*)vector_at(points, i) - pointsOfGrid]) == 0);
}

void grid_mutex_unlock_partial(vector_t *points, size_t max)
{
	size_t i = 0;
	for (; i < max; i++)
		assert(pthread_mutex_unlock(&grid_mutex[(long*)vector_at(points, i) - pointsOfGrid]) == 0);
}

void grid_mutex_free() {
	free(grid_mutex);
}


#include "mutex.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>

static pthread_mutex_t queue_mutex;
static pthread_mutex_t *grid_mutex;
static long* pointsOfGrid;

// TODO: Assert that all  mutex operations return 0 (sucessfull)

void queue_mutex_init()
{
	pthread_mutex_init(&queue_mutex, NULL);
}

void queue_mutex_lock()
{
	pthread_mutex_lock(&queue_mutex);
}

void queue_mutex_unlock()
{
	pthread_mutex_unlock(&queue_mutex);
}

void grid_mutex_init(long size, long* points;)
{
	pointsOfGrid = points;
	int i;
	grid_mutex = malloc(sizeof(pthread_mutex_t) * size);
	for (i = 0; i < size; i++)
		pthread_mutex_init(&grid_mutex[i], NULL);
}

void grid_mutex_lock(vector_t *points)
{
	int i;
	int size = vector_getSize(points);
	for (i = 0; i < size; i++)
		pthread_mutex_lock(&grid_mutex[pointsOfGrid - vector_at(points, i)]);
}

void grid_mutex_unlock(vector_t *points)
{
	int i;
	int size = vector_getSize(points);
	for (i = 0; i < size; i++)
		pthread_mutex_unlock(&grid_mutex[pointsOfGrid - vector_at(points, i)]);
}

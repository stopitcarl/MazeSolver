#include "mutex.h"
#include <stdio.h>

static pthread_mutex_t queue_mutex;
static pthread_mutex_t grid_mutex;

// TODO: Assert that all  mutex operations return 0 (sucessfull)

void  queue_mutex_init()
{
	pthread_mutex_init(&queue_mutex, NULL);
}

void queue_mutex_lock()
{
	printf("Locking queue\n");
	pthread_mutex_lock(&queue_mutex);
}

void queue_mutex_unlock()
{
	printf("Unlocking queue\n");
	pthread_mutex_unlock(&queue_mutex);
}


void grid_mutex_init()
{

	pthread_mutex_init(&grid_mutex, NULL);
}

void grid_mutex_lock()
{
	printf("Locking grid\n");
	pthread_mutex_lock(&grid_mutex);
}

void grid_mutex_unlock()
{
	printf("Unlocking grid\n");
	pthread_mutex_unlock(&grid_mutex);
}

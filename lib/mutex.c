#include "mutex.h"

static pthread_mutex_t queue_mutex;

// TODO: Assert that all  mutex operations return 0 (sucessfull)

void  queue_init()
{
    pthread_mutex_init(&queue_mutex, NULL);
}

void queue_lock()
{    
    pthread_mutex_lock(&queue_mutex);
}

void queue_unlock()
{
    pthread_mutex_unlock(&queue_mutex);
}
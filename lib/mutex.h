#ifndef MUTEX_H
#define MUTEX_H 1


#endif /* MUTEX_H */

#include <pthread.h>

// TODO: Change function names for consistency

void queue_mutex_init();
void queue_mutex_lock ();
void queue_mutex_unlock();


void grid_mutex_init();
void grid_mutex_lock (vector_t* points);
void grid_mutex_unlock(vector_t* points);


/* =============================================================================
 *
 * End of pair.h
 *
 * =============================================================================
 */

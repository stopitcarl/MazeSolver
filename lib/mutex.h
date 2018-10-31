#ifndef MUTEX_H
#define MUTEX_H 1


#endif /* MUTEX_H */

#include <pthread.h>

// TODO: Change function names for consistency

void queue_init();
void queue_lock ();
void queue_unlock();


void grid_mutex_init();
void grid_mutex_lock ();
void grid_mutex_unlock();


/* =============================================================================
 *
 * End of pair.h
 *
 * =============================================================================
 */

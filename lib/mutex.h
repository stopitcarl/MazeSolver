#ifndef MUTEX_H
#define MUTEX_H 1

#define NUMBEROFTRIES 5
#define SLEEPTIME (const struct timespec[]) { {0, 150L} }, NULL



#endif /* MUTEX_H */

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include "vector.h"
#include "../CircuitRouter-ParSolver/grid.h"
#include "vector.h"


void queue_mutex_init();
void queue_mutex_lock();
void queue_mutex_unlock();

void path_mutex_init();
void path_mutex_lock();
void path_mutex_unlock();

void grid_mutex_init(grid_t* grid);
int grid_mutex_lock(vector_t* points);
void grid_mutex_unlock(vector_t* points);
void grid_mutex_unlock_partial(vector_t *points, size_t max);
void grid_mutex_free();


/* =============================================================================
 *
 * End of pair.h
 *
 * =============================================================================
 */

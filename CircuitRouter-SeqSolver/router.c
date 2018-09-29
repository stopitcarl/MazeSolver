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
 * router.c
 *
 * =============================================================================
 */


#include <assert.h>
#include <stdlib.h>
#include "coordinate.h"
#include "grid.h"
#include "lib/queue.h"
#include "router.h"
#include "lib/vector.h"


typedef enum momentum {
    MOMENTUM_ZERO = 0,
    MOMENTUM_POSX = 1,
    MOMENTUM_POSY = 2,
    MOMENTUM_POSZ = 3,
    MOMENTUM_NEGX = 4,
    MOMENTUM_NEGY = 5,
    MOMENTUM_NEGZ = 6
} momentum_t;

typedef struct point {
    long x;
    long y;
    long z;
    long value;
    momentum_t momentum;
} point_t;

point_t MOVE_POSX = { 1,  0,  0,  0, MOMENTUM_POSX};
point_t MOVE_POSY = { 0,  1,  0,  0, MOMENTUM_POSY};
point_t MOVE_POSZ = { 0,  0,  1,  0, MOMENTUM_POSZ};
point_t MOVE_NEGX = {-1,  0,  0,  0, MOMENTUM_NEGX};
point_t MOVE_NEGY = { 0, -1,  0,  0, MOMENTUM_NEGY};
point_t MOVE_NEGZ = { 0,  0, -1,  0, MOMENTUM_NEGZ};


/* =============================================================================
 * router_alloc
 * =============================================================================
 */
router_t* router_alloc (long xCost, long yCost, long zCost, long bendCost){
    router_t* routerPtr;

    routerPtr = (router_t*)malloc(sizeof(router_t));
    if (routerPtr) {
        routerPtr->xCost = xCost;
        routerPtr->yCost = yCost;
        routerPtr->zCost = zCost;
        routerPtr->bendCost = bendCost;
    }

    return routerPtr;
}


/* =============================================================================
 * router_free
 * =============================================================================
 */
void router_free (router_t* routerPtr){
    free(routerPtr);
}


/* =============================================================================
 * expandToNeighbor
 * =============================================================================
 */
static void expandToNeighbor (grid_t* myGridPtr, long x, long y, long z, long value, queue_t* queuePtr){
    if (grid_isPointValid(myGridPtr, x, y, z)) {
        long* neighborGridPointPtr = grid_getPointRef(myGridPtr, x, y, z);
        long neighborValue = *neighborGridPointPtr;
        if (neighborValue == GRID_POINT_EMPTY) {
            (*neighborGridPointPtr) = value;
            queue_push(queuePtr, (void*)neighborGridPointPtr);
        } else if (neighborValue != GRID_POINT_FULL) {
            /* We have expanded here before... is this new path better? */
            if (value < neighborValue) {
                (*neighborGridPointPtr) = value;
                queue_push(queuePtr, (void*)neighborGridPointPtr);
            }
        }
    }
}


/* =============================================================================
 * doExpansion
 * =============================================================================
 */
static bool_t doExpansion (router_t* routerPtr, grid_t* myGridPtr, queue_t* queuePtr, coordinate_t* srcPtr, coordinate_t* dstPtr){
    long xCost = routerPtr->xCost;
    long yCost = routerPtr->yCost;
    long zCost = routerPtr->zCost;

    /*
     * Potential Optimization: Make 'src' the one closest to edge.
     * This will likely decrease the area of the emitted wave.
     */

    queue_clear(queuePtr);
    long* srcGridPointPtr = grid_getPointRef(myGridPtr, srcPtr->x, srcPtr->y, srcPtr->z);
    queue_push(queuePtr, (void*)srcGridPointPtr);
    grid_setPoint(myGridPtr, srcPtr->x, srcPtr->y, srcPtr->z, 0);
    grid_setPoint(myGridPtr, dstPtr->x, dstPtr->y, dstPtr->z, GRID_POINT_EMPTY);
    long* dstGridPointPtr = grid_getPointRef(myGridPtr, dstPtr->x, dstPtr->y, dstPtr->z);
    bool_t isPathFound = FALSE;

    while (!queue_isEmpty(queuePtr)) {

        long* gridPointPtr = (long*)queue_pop(queuePtr);
        if (gridPointPtr == dstGridPointPtr) {
            isPathFound = TRUE;
            break;
        }

        long x;
        long y;
        long z;
        grid_getPointIndices(myGridPtr, gridPointPtr, &x, &y, &z);
        long value = (*gridPointPtr);

        /*
         * Check 6 neighbors
         *
         * Potential Optimization: Only need to check 5 of these
         */
        expandToNeighbor(myGridPtr, x+1, y,   z,   (value + xCost), queuePtr);
        expandToNeighbor(myGridPtr, x-1, y,   z,   (value + xCost), queuePtr);
        expandToNeighbor(myGridPtr, x,   y+1, z,   (value + yCost), queuePtr);
        expandToNeighbor(myGridPtr, x,   y-1, z,   (value + yCost), queuePtr);
        expandToNeighbor(myGridPtr, x,   y,   z+1, (value + zCost), queuePtr);
        expandToNeighbor(myGridPtr, x,   y,   z-1, (value + zCost), queuePtr);

    } /* iterate over work queue */

    return isPathFound;
}


/* =============================================================================
 * traceToNeighbor
 * =============================================================================
 */
static void traceToNeighbor (grid_t* myGridPtr, point_t* currPtr, point_t* movePtr, bool_t useMomentum, long bendCost, point_t* nextPtr){
    long x = currPtr->x + movePtr->x;
    long y = currPtr->y + movePtr->y;
    long z = currPtr->z + movePtr->z;

    if (grid_isPointValid(myGridPtr, x, y, z) &&
        !grid_isPointEmpty(myGridPtr, x, y, z) &&
        !grid_isPointFull(myGridPtr, x, y, z))
    {
        long value = grid_getPoint(myGridPtr, x, y, z);
        long b = 0;
        if (useMomentum && (currPtr->momentum != movePtr->momentum)) {
            b = bendCost;
        }
        if ((value + b) <= nextPtr->value) { /* '=' favors neighbors over current */
            nextPtr->x = x;
            nextPtr->y = y;
            nextPtr->z = z;
            nextPtr->value = value;
            nextPtr->momentum = movePtr->momentum;
        }
    }
}


/* =============================================================================
 * doTraceback
 * =============================================================================
 */
static vector_t* doTraceback (grid_t* gridPtr, grid_t* myGridPtr, coordinate_t* dstPtr, long bendCost){
    vector_t* pointVectorPtr = vector_alloc(1);
    assert(pointVectorPtr);

    point_t next;
    next.x = dstPtr->x;
    next.y = dstPtr->y;
    next.z = dstPtr->z;
    next.value = grid_getPoint(myGridPtr, next.x, next.y, next.z);
    next.momentum = MOMENTUM_ZERO;

    while (1) {

        long* gridPointPtr = grid_getPointRef(gridPtr, next.x, next.y, next.z);
        vector_pushBack(pointVectorPtr, (void*)gridPointPtr);
        grid_setPoint(myGridPtr, next.x, next.y, next.z, GRID_POINT_FULL);

        /* Check if we are done */
        if (next.value == 0) {
            break;
        }
        point_t curr = next;

        /*
         * Check 6 neighbors
         *
         * Potential Optimization: Only need to check 5 of these
         */
        traceToNeighbor(myGridPtr, &curr, &MOVE_POSX, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_POSY, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_POSZ, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_NEGX, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_NEGY, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_NEGZ, TRUE, bendCost, &next);

        /*
         * Because of bend costs, none of the neighbors may appear to be closer.
         * In this case, pick a neighbor while ignoring momentum.
         */
        if ((curr.x == next.x) &&
            (curr.y == next.y) &&
            (curr.z == next.z))
        {
            next.value = curr.value;
            traceToNeighbor(myGridPtr, &curr, &MOVE_POSX, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_POSY, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_POSZ, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_NEGX, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_NEGY, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_NEGZ, FALSE, bendCost, &next);

            if ((curr.x == next.x) &&
                (curr.y == next.y) &&
                (curr.z == next.z))
            {
                vector_free(pointVectorPtr);
                return NULL; /* cannot find path */
            }
        }
    }

    return pointVectorPtr;
}


/* =============================================================================
 * router_solve
 * =============================================================================
 */
void router_solve (void* argPtr){

    router_solve_arg_t* routerArgPtr = (router_solve_arg_t*)argPtr;
    router_t* routerPtr = routerArgPtr->routerPtr;
    maze_t* mazePtr = routerArgPtr->mazePtr;
    vector_t* myPathVectorPtr = vector_alloc(1);
    assert(myPathVectorPtr);

    queue_t* workQueuePtr = mazePtr->workQueuePtr;
    grid_t* gridPtr = mazePtr->gridPtr;
    grid_t* myGridPtr = grid_alloc(gridPtr->width, gridPtr->height, gridPtr->depth);
    assert(myGridPtr);
    long bendCost = routerPtr->bendCost;
    queue_t* myExpansionQueuePtr = queue_alloc(-1);

    /*
     * Iterate over work list to route each path. This involves an
     * 'expansion' and 'traceback' phase for each source/destination pair.
     */
    while (1) {

        pair_t* coordinatePairPtr;
        if (queue_isEmpty(workQueuePtr)) {
            coordinatePairPtr = NULL;
        } else {
            coordinatePairPtr = (pair_t*)queue_pop(workQueuePtr);
        }
        if (coordinatePairPtr == NULL) {
            break;
        }

        coordinate_t* srcPtr = coordinatePairPtr->firstPtr;
        coordinate_t* dstPtr = coordinatePairPtr->secondPtr;

        pair_free(coordinatePairPtr);

        bool_t success = FALSE;
        vector_t* pointVectorPtr = NULL;

        grid_copy(myGridPtr, gridPtr); /* create a copy of the grid, over which the expansion and trace back phases will be executed. */
        if (doExpansion(routerPtr, myGridPtr, myExpansionQueuePtr,
                         srcPtr, dstPtr)) {
            pointVectorPtr = doTraceback(gridPtr, myGridPtr, dstPtr, bendCost);
            if (pointVectorPtr) {
                grid_addPath_Ptr(gridPtr, pointVectorPtr);

                success = TRUE;
            }
        }

        if (success) {
            bool_t status = vector_pushBack(myPathVectorPtr,(void*)pointVectorPtr);
            assert(status);
        }

    }

    /*
     * Add my paths to global list
     */
    list_t* pathVectorListPtr = routerArgPtr->pathVectorListPtr;
    list_insert(pathVectorListPtr, (void*)myPathVectorPtr);

    grid_free(myGridPtr);
    queue_free(myExpansionQueuePtr);
}


/* =============================================================================
 *
 * End of router.c
 *
 * =============================================================================
 */

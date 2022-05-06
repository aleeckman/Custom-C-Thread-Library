#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "uthread.h"
#include "queue.h"
#include "sem.h"
#include "private.h"

#define ERROR   -1
#define NO_ERROR 0

/*
 * semaphore - user level data type to for Synchronzing Access
 *  
 * This abstract data struture is responsible for controlling
 * access for shared resources in a multithreading system.
 * The most significant dilemma of a multithreading system
 * is there could be a Race Condition, if the threads are
 * trying to get access to shared objects. 
 * 
 * In light of this, synchonization variables, which allows 
 * mutual excusion, should be implemented in the thread library, 
 * so that only one of the thread can get access to the resources.
 * 
 * This abstract data structure is holding the following info:
 * 
 * 1. resources_avail           : a.k.a 'count', keep track of # of 
 *                                resources still available to threads
 * 
 * 2. block_threads             : a queue which stores threads that 
 *                                are being blocked.
 * 
 * 3. num_of_blocked_threads    : # of blocked threads stored
 *                                in block_threads;
 */

typedef struct semaphore 
{
    size_t resources_avail;
    queue_t blocked_threads;
    int num_of_blocked_threads;

} semaphore;

sem_t sem_create(size_t count)
{
    /* Initialize space for the new semaphore */
    sem_t sem = malloc(sizeof(semaphore));

    /* If no space available, return NULL */
    if(sem == NULL)
        return NULL;

    /* Create the semaphore */
    sem->blocked_threads        = queue_create();
    sem->resources_avail        = count;
    sem->num_of_blocked_threads = 0;

    return sem;
}

int sem_destroy(sem_t sem)
{
    preempt_disable();

    /* Check if sem is NULL and if the blocked thread queue is empty */
    if(sem == NULL || queue_length(sem->blocked_threads) > 0)
        return ERROR;

    /* Free the queue's space */
    if(queue_destroy(sem->blocked_threads))
        return ERROR;

    /* Free the remaining allocated space for the semaphore */
    free(sem);
    
    preempt_enable();

    return NO_ERROR;
}

int sem_down(sem_t sem)
{
    preempt_disable();

    /* Semaphore being passed is NULL, execution failed */
    if(sem == NULL)
        return ERROR;

    /* If no resources are available block the current thread */
    while(sem->resources_avail == 0)
    {
        sem->num_of_blocked_threads++;
        queue_enqueue(sem->blocked_threads, (void*)uthread_current());
        uthread_block();
    }

    /* If resources are available, take one of those resources. */
    if(sem->resources_avail > 0)
    {
        sem->resources_avail -= 1;
    }

    preempt_enable();

    return NO_ERROR;
}

int sem_up(sem_t sem)
{
    preempt_disable();

    /* Check to make sure the semaphore being passed is not NULL */
    if(sem == NULL)
        return ERROR;

    struct uthread_tcb *thread_to_unblock;

    /* Put one of the resources back and allow other threads to take it */
    sem->resources_avail += 1;

    /* If there are blocked threads, unblock the first one in the queue
       Move this queue to the front of the queue in uthread.c */
    if(sem->num_of_blocked_threads > 0)
    {
        sem->num_of_blocked_threads--;
        queue_dequeue(sem->blocked_threads, (void **) &thread_to_unblock);
        uthread_unblock(thread_to_unblock);
    }

    preempt_enable();

    return NO_ERROR;
}
#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>

#include "queue.h"
#include "private.h"
#include "uthread.h"

#define NO_ERROR     0
#define ERROR_FOUND -1

typedef struct uthread_tcb * uthread_tcb_t;

/*
 * ready_q : non-user level queue data structure
 *  
 * This data struture is responsible for holding
 * all ready threads. Thus helps manage all 
 * threads that are neither Running nor Blocked
 * 
 * Implementation of such data structure allows
 * O(1) time complexity for storing and extracting
 * information through enqueue and dequeue.
 */
queue_t ready_q;

/*
 * blocked_q : non-user level queue data structure
 *  
 * This data struture is responsible for holding
 * all blocked threads. Thus helps manage all 
 * threads that are neither Ready nor Running
 * 
 * Implementation of such data structure allows
 * O(1) time complexity for storing and extracting
 * information through enqueue and dequeue.
 */
queue_t blocked_q;

/*
 * main_tcb : non-user level Thread Control Block
 *  
 * This struct is responsible for controlling 
 * the overrall instruction of thread_start().
 * It does the following things. This thread
 * is as known as "Main Execution Thread".
 * 
 * 1. Distinguish if there is any Ready threads. 
 * 2. Continuing execution of the library if 
 *    situation permitted.
 * 3. Responsible for Multithreading Scheduling
 *    through an infinite loop, that is breakable 
 *    when there is no any Ready threads.
 * 
 * When this thread is assigned to be the Running
 * thread (a.k.a current_tcb), it will called 
 * uthread_yield() immediately and switch context
 * with the next available Ready thread
 */
uthread_tcb_t main_tcb;

/*
 * current_tcb : non-user level Thread Control Block
 *  
 * This struct is responsible for storing all info
 * of current running thread. The current running 
 * thread is the unique thread that is in Running
 * state. 
 * 
 * There should not be and it should be impossible 
 * for another thread to be in the Running state,
 * since this lib doesn't support Multi CPU 
 * Concurrency and Synchronization.
 */
uthread_tcb_t current_tcb;

/*
 * uthread_tcb : non-user level Thread Control Block
 *  
 * This data struct is responsible for storing all 
 * information of a thread, which are :
 * 1. Thread's ID
 * 2. Thread's State (Ready, Running, Blocked, Exit)
 * 3. A Pointer to top of the assigned Stack
 * 4. Thread Context
 */
typedef struct uthread_tcb
{
    int tid;                
    unsigned state;       
    void *stack;           
    uthread_ctx_t ctx;    

} uthread_tcb;

/* num_of_threads -- The total number of threads 
 *                   currently in either Ready, 
 *                   Blocked, or Running states
 */
int num_of_threads;

/* Thread_State -- State of a Thread
 *
 * This type store all possible states of thread, 
 * which are described below :
 *  
 * Running -- Executing assigned tasks. There can
 *            only be one thread to be in Running 
 *            state. The corresponding thread's
 *            address will be stored in current_tcb.
 * 
 * Ready   -- Available to be selected and executing
 *            tasks. The Ready threads are stored
 *            in ready_q.
 *              
 * Blocked -- Not Available to be selected and execute
 *            any tasks unless being unblock(). 
 * 
 * Exit    -- The thread no longer exists and has 
 *            finished it's executions.
 */
enum Thread_State {
    RUNNING,
    READY,
    BLOCKED,
    EXIT
};

void uthread_yield(void)
{
	preempt_disable();

	/* No threads waiting so return and finish execution instead. */
	if(queue_length(ready_q) == 0)
		return;
	
	/* 1. When the running thread yields, it shd be enqueue */
	current_tcb->state = READY;
	queue_enqueue(ready_q, current_tcb);
	uthread_tcb_t prev_tcb = current_tcb;

	// 2. Then, the first thread in the queue shd be dequeue
	uthread_tcb_t next_tcb = malloc(sizeof(uthread_tcb));

	queue_dequeue(ready_q, (void**) &next_tcb);	

	/* 3. Now, the dequeued thread (next_tcb) is the running thread
	      We shd set it to be current_tcb (current running tcb) */
	next_tcb->state    = RUNNING;
	current_tcb        = next_tcb;

	/* 4. Now, run the task assigned for next_tcb */
	uthread_ctx_switch(&prev_tcb->ctx, &current_tcb->ctx);

	preempt_enable();
}

void uthread_exit(void)
{
	preempt_disable();

	uthread_tcb_t next_tcb = malloc(sizeof(uthread_tcb));

	/* Another thread exists that is ready */
	if(queue_length(ready_q) > 0)
	{
		queue_dequeue(ready_q, (void**) &next_tcb);
	}

	/* No other threads exist in the ready queue, 
	return to uthread_start() and execute main thread */
	else
	{
		next_tcb = main_tcb;
	}

	uthread_tcb_t temp = current_tcb;

	/* Destroy Current Running Thread */
	current_tcb->state = EXIT;
	uthread_ctx_destroy_stack(current_tcb->stack);

	/* Current Running Thread will be the next thread in the ready queue */
	current_tcb = next_tcb;
	num_of_threads--;

	uthread_ctx_switch(&temp->ctx, &next_tcb->ctx);

	preempt_enable();
}

int uthread_create(uthread_func_t func, void *arg)
{
	preempt_disable();

	/* Creating the new thread */
	uthread_ctx_t *thread_ctx  = malloc(sizeof(uthread_ctx_t));
	uthread_tcb_t new_thread_t = malloc(sizeof(uthread_tcb));
	new_thread_t->tid          = num_of_threads;
	new_thread_t->stack        = uthread_ctx_alloc_stack();
	new_thread_t->state        = READY;
	new_thread_t->ctx          = *thread_ctx;

	/* initalize new thread's execution context */
	if(uthread_ctx_init(&new_thread_t->ctx, new_thread_t->stack, func, arg))
		return ERROR_FOUND;

	/* A new thread is successfully created, add it into the ready queue */
	queue_enqueue(ready_q, new_thread_t);
	num_of_threads++;

	preempt_enable();

	return NO_ERROR;
}

int uthread_start(uthread_func_t func, void *arg)
{
	/* The queue shd be initialize when the lib is created */
	ready_q   = queue_create();
	blocked_q = queue_create();

	/* Initialize the main thread */
	uthread_tcb_t main_thread = malloc(sizeof(uthread_tcb));
	uthread_ctx_t *main_ctx   = malloc(sizeof(uthread_ctx_t));
	main_thread->tid          = 0;
	main_thread->state        = READY;
	main_thread->stack        = uthread_ctx_alloc_stack();
	main_thread->ctx          = *main_ctx;

	/* Initialize main thread's execution context */
	if(uthread_ctx_init(&main_thread->ctx, main_thread->stack, NULL, NULL))
		return ERROR_FOUND;

	num_of_threads++;

	/* Store the address of main_tcb globally */
	main_tcb = main_thread;

	/* At this moment, the main thread is the current Running thread */
	current_tcb = main_tcb;

	/* The function preempt_start() should be called when the 
	   uthread library is initializing and sets up preemption. */
	preempt_start();

	/* Create an initial thread and start the multithreading process */
	if(uthread_create(func, arg))
		return ERROR_FOUND;

	/* Start Multithread Scheduling by executing an infinite loop 
	   the loop will break if there is no more threads Ready */
	while(queue_length(ready_q))
	{	
		uthread_yield();
	}

	/* Destroy the queue before leaving the library */
	queue_destroy(ready_q);
	queue_destroy(blocked_q);

	/* preempt_stop() should be called before uthread_start() return */
	preempt_stop();

	/* No problems were detected so report perfect execution */
	return NO_ERROR;
}

void uthread_block(void)
{
	preempt_disable();

	/* Add current running thread to block queue */
	current_tcb->state = BLOCKED;
	queue_enqueue(blocked_q, current_tcb);

	/* When current_tcb is blocked, we shd switch to next_tcb */
	uthread_tcb_t next_tcb = malloc(sizeof(uthread_tcb));

	/* Get next_tcb and set it to be current Running Thread */
	queue_dequeue(ready_q, (void**) &next_tcb);
	next_tcb->state = RUNNING;
	uthread_tcb_t temp = current_tcb;
	current_tcb = next_tcb;

	uthread_ctx_switch(&temp->ctx, &current_tcb->ctx);

	preempt_enable();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	preempt_disable();
	
	int len = queue_length(blocked_q);
	uthread_tcb_t temp = malloc(sizeof(uthread_tcb));

	/* Keep iterating the block_q until we find uthread
	   or until len == 0 */
	while(len) 
	{
		/* Store extracted thread to temp */
		queue_dequeue(blocked_q, (void **) &temp);

		/* Enqueue uthread to the back of the Ready_q */
		if(temp->tid == uthread->tid) {
			uthread->state = READY;
			queue_enqueue(ready_q, uthread);
			break;
		}
		
		/* Keep looping since temp is not uthread */
		queue_enqueue(blocked_q, temp);
		len--;
	}

	preempt_enable();
}

uthread_tcb_t uthread_current(void)
{
	return current_tcb;
}

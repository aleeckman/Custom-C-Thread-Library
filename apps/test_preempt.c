/* 
 * Break Infinite Loop Test
 *
 * This test checks if preemption implemented correctly.
 * Creating a delay long enough to trigger preemption by
 * using a while(true) loop.
 * 
 * If preemption being trigger, thread1 will yield() to
 * thread2, hence setting thread2_ran to false, thus
 * breaking the infinite loop inside thread1.
 */

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "private.h"
#include "uthread.h"

bool thread2_ran = false;

static void thread2(void *arg)
{
    printf("This is thread 2.\n");
    printf("I was successfully preempted by preempt.c\n");
    thread2_ran = true;
	fflush(stdout);
}

static void thread1(void *arg) 
{
    printf("This is thread 1.\n");
    printf("I am going to attempt to hold the CPU for a long time.\n");  
    uthread_create(thread2, NULL);
    
	/* It seems that Signaction has some problem when 
	   it comes to deal with printf() or write(). 
	   keep fflushing STDOUT can solve such dilemma. */
    while(!thread2_ran) {
		fflush(stdout);
	};

	/* Test Passed */
	printf("Successfully return from thread2!\n");
	printf("Bye ... Cruel Cruel World ... \n");
	printf("Pass\n");
}

static void test_break_infinite_loop(void) {
	printf("    ***  Test: Break_Infinite_Loop ***\n\n");
    uthread_start(thread1, NULL);
}

int main(void) 
{
	test_break_infinite_loop();
}
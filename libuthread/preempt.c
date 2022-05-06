#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 10 * HZ ms = 1000 ms = 0.01s 
 */
#define HZ 100

/* 
 * timer -- non user level data struct 
 * 
 * This data struct is designed to use along with
 * a signal handler. It will send a SIGVTARLM signal 
 * to the system per 0.01s, which is 1000 ms. Hence, 
 * 100 SIGVTARLM singal will be sent with in 1 second.
 * 
 * Implementation of such data struct allows RR
 * (Robin-Round) scheduling, In RR scheduling, all
 * threads will be given a fixed amount of time, which
 * is 1000ms here. Such scheduling prevents thread from
 * holding the resources for too long, while at the same
 * time decreasing average waiting time for the thread.  
 * 
 * When the timer is set to stop, its it_value and 
 * it_interval will be set to zero
 */
struct itimerval timer;

/* 
 * signal_handler -- non user level data struct 
 * 
 * This data struct is designed to implement Robin-Round
 * scheduling. It is planned to receive SIGVTARLM signal
 * from the system. Once the SIGVTARLM signal received,
 * this means the amount of time planned for Current
 * Running Thread expires. The multithreading scheduler
 * will context switch to the next available threads.
 * 
 * Notice that signal handler can also enable and disable.
 * 
 * When being disabled, SIGVTARLM signal received can no
 * longer trigger context switch.
 * When being enabled, SIGVTARLM signal received can again
 * trigger context switch. The signal handler is in the
 * such state when being start.
 * 
 * When the signal handler is stops, it will ignore the 
 * SIGVTARLM signal, and thus can no longer be able 
 * to trigger to context switch.
 */
struct sigaction signal_handler; 

void preempt_disable(void)
{
    /* Block SIGVTALRM in Signal Handler*/
    sigemptyset(&signal_handler.sa_mask);
    sigaddset(&signal_handler.sa_mask, SIGVTALRM);

    /* Change Block Signal */
    sigprocmask(SIG_SETMASK, &signal_handler.sa_mask, NULL);
    sigemptyset(&signal_handler.sa_mask);
}

void preempt_enable(void)
{
    /* Unblock SIGVTALRM in Signal Handler*/
    sigemptyset(&signal_handler.sa_mask);
    sigaddset(&signal_handler.sa_mask, SIGVTALRM);

    /* Change Unblock Signal */
    sigprocmask(SIG_UNBLOCK, &signal_handler.sa_mask, NULL);
    sigemptyset(&signal_handler.sa_mask);
}

void response_handler() 
{
    /* When receive signal move to next tcb */
    uthread_yield();
}

void preempt_start(void)
{
    /* Setting the block Signal to be SIGVTALRM */
    sigemptyset(&signal_handler.sa_mask);
    sigaddset(&signal_handler.sa_mask, SIGVTALRM);

    /* When being interrupted, execute response_handler() */
    signal_handler.sa_handler = &response_handler;

    /* Create the Signal Handler */
    sigaction(SIGVTALRM, &signal_handler, NULL);

    /* Create the Timer */
    timer.it_interval.tv_sec  = 0;
    timer.it_interval.tv_usec = 10 * HZ;
    timer.it_value.tv_sec     = 0;
    timer.it_value.tv_usec    = 10 * HZ;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void preempt_stop(void)
{
    /* A timer which is set to zero (it_value is zero or
       the timer expires and it_interval is zero) stops. 
       https://linux.die.net/man/2/setitimer */
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_usec    = 0;

    /* sa_handler specifies the action to be associated 
       with signum and may be SIG_DFL for the default action,
       SIG_IGN to ignore this signal, or a pointer to a signal 
       handling function.
       https://man7.org/linux/man-pages/man2/sigaction.2.html */
    signal_handler.sa_handler = SIG_IGN;
    sigaction(SIGVTALRM, &signal_handler, NULL);
}

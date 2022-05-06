#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

#define TEST_ASSERT(assert)                \
do {                                    \
    printf("ASSERT: " #assert " ... ");    \
    if (assert) {                        \
        printf("PASS\n");                \
    } else    {                            \
        printf("FAIL\n");                \
        exit(1);                        \
    }                                    \
} while(0)

int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
queue_t q;

/* Create */
void test_create(void)
{
    fprintf(stderr, "*** TEST create ***\n");

    TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
    int *ptr;
    fprintf(stderr, "*** TEST queue_simple ***\n");

    q = queue_create();
    queue_enqueue(q, &data[0]);
    queue_dequeue(q, (void**)&ptr);

    TEST_ASSERT(ptr == &data[0]);
}

/* Enqueue/Dequeue Multiple */
void test_queue_dequeue_multiple(void)
{
    int *ptr1, *ptr2, *ptr3;
    fprintf(stderr, "*** TEST queue_dequeue_multiple ***\n");

    for(int i = 0; i < 10; i++) {
        queue_enqueue(q, &data[i]);
    }

    queue_dequeue(q, (void**)&ptr1);
    TEST_ASSERT(ptr1 == &data[0]);

    queue_dequeue(q, (void**)&ptr2);
    TEST_ASSERT(ptr2 == &data[1]);

    queue_dequeue(q, (void**)&ptr3);
    TEST_ASSERT(ptr3 == &data[2]);
}

/* Delete */
void test_queue_delete(void) {
    fprintf(stderr, "*** TEST queue_delete ***\n");
    int data2[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    queue_t q2;
    
    q2 = queue_create();
    
    for(int i = 0; i < 10; i++) {
        queue_enqueue(q2, &data2[i]);
    }

    TEST_ASSERT(queue_delete(q, &data[9]) == 0);
    TEST_ASSERT(queue_delete(q, &data[6]) == 0);
}

/* Inc item */
static void inc_item(void *d)
{
    int *a = (int*) d;
    if(*a == 5)
        queue_delete(q, d);
    else
        *a += 1;
}


/* Queue_iterate */
void test_queue_iterate(void) {
    
    fprintf(stderr, "*** TEST queue_iterate ***\n");
    
    queue_iterate(q, inc_item);
    printf("queue length: %d\n", queue_length(q));
    TEST_ASSERT(data[5] == 7);
    TEST_ASSERT(queue_length(q) == 4);
}

/* Destroy */
void test_queue_destroy(void) {
    int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    fprintf(stderr, "*** TEST queue_destroy ***\n");

    for(int i = 0; i < 10; i++) {
            queue_enqueue(q, &data[i]);
        }
    TEST_ASSERT(queue_destroy(q) == 0);
}

int main(void)
{
    /* First create the queue */
    test_create();

    /* Enqueue and dequeue one data point */
    test_queue_simple();

    /* Using the global data set */
    test_queue_dequeue_multiple();
    test_queue_delete();
    test_queue_iterate();
    test_queue_destroy();

    return 0;
}


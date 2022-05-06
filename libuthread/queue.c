#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

#define ONE_NODE         1

#define ERROR_FOUND     -1
#define NO_ERROR         0

/* 
 * queue_node - Node type
 * 
 * A queue_node is a node inside a queue. It stores data,
 * address to previous of next node in the queue.
 * 
*/
typedef struct queue_node {
    void* data_in_node;                  // Store the address of the node's data
    struct queue_node *next_in_queue;    // Point to next node in the queue
    struct queue_node *prev_in_queue;    // Point to previous node in the queue
} queue_node;


/*
 * queue_t - Queue type
 *
 * A queue is a FIFO data structure. Data items are enqueued one after the
 * other.  When dequeueing, the queue must returned the oldest enqueued item
 * first and so on.
 *
 * Apart from delete and iterate operations, all operations should be O(1).
 */
typedef struct queue {
    queue_node *first_in_queue;          // First Node in the queue
    queue_node *last_in_queue;           // Last  Node in the queue
    int num_of_nodes;                    // # of nodes present in the queue 
} queue;

/*
 * queue_create - Allocate an empty queue
 *
 * Create a new object of type 'struct queue' and return its address.
 *
 * Return: Pointer to new empty queue. NULL in case of failure when allocating
 * the new queue.
 */
queue_t queue_create(void)
{
    queue_t new_queue = malloc(sizeof(queue));

    new_queue->first_in_queue = NULL;
    new_queue->last_in_queue  = NULL;
    new_queue->num_of_nodes   = 0;

    return new_queue;
}

/*
 * queue_destroy - Deallocate a queue
 * @queue: Queue to deallocate
 *
 * Deallocate the memory associated to the queue object pointed by @queue.
 *
 * Return: -1 if @queue is NULL or if @queue is not empty. 0 if @queue was
 * successfully destroyed.
 */
int queue_destroy(queue_t queue)
{
    if(queue == NULL || queue->num_of_nodes == 0)
        return ERROR_FOUND;

    queue_node *node_to_destroy = queue->first_in_queue;
    queue_node *next_node = node_to_destroy->next_in_queue;

    /* Free queue_node inside the queue */
    while(queue->num_of_nodes > 0) {
        free(node_to_destroy);
        node_to_destroy = NULL;
        queue->num_of_nodes -= ONE_NODE;

        // there is still nodes inside the queue, keep iterating
        if(queue->num_of_nodes > 0) {
            node_to_destroy = next_node;
            next_node = node_to_destroy->next_in_queue;
        }
    }

    /* Now, free the queue_t */
    queue->first_in_queue = NULL;
    queue->last_in_queue  = NULL;
    free(queue);
    queue = NULL;

    return NO_ERROR;
}

/*
 * queue_enqueue - Enqueue data item
 * @queue: Queue in which to enqueue item
 * @data: Address of data item to enqueue
 *
 * Enqueue the address contained in @data in the queue @queue.
 *
 * Return: -1 if @queue or @data are NULL, or in case of memory allocation error
 * when enqueing. 0 if @data was successfully enqueued in @queue.
 */
int queue_enqueue(queue_t queue, void *data)
{
    queue_node *node_to_enqueue = malloc(sizeof(queue_node));

    /* Check for problems with data, memory allocation, and passed queue */
    if(node_to_enqueue == NULL || queue == NULL || data == NULL)
    {
        return ERROR_FOUND;
    }

    /* If no problem exists with data, memory allocation, and passed queue, proceed*/
    node_to_enqueue->data_in_node = data;

    /* We know that there will be no other node after this new one yet so: */
    node_to_enqueue->next_in_queue = NULL;
    node_to_enqueue->prev_in_queue = NULL;

    /* Queue is empty */
    if(queue->num_of_nodes == 0)
    {
        /* Add node to the queue, since it is empty, this becomes the first and last element in the queue*/
        queue->first_in_queue = node_to_enqueue;
        queue->last_in_queue = node_to_enqueue;
    }

    /* Queue is not empty */
    else
    {
        node_to_enqueue->prev_in_queue = queue->last_in_queue;
        queue->last_in_queue->next_in_queue = node_to_enqueue;
        queue->last_in_queue = node_to_enqueue;
    }

    /* Regardless of whether it is the first node in the queue or not, a new node has been added */
    queue->num_of_nodes += ONE_NODE;

    return NO_ERROR;
}

/*
 * queue_dequeue - Dequeue data item
 * @queue: Queue in which to dequeue item
 * @data: Address of data pointer where item is received
 *
 * Remove the oldest item of queue @queue and assign this item (the value of a
 * pointer) to @data.
 *
 * Return: -1 if @queue or @data are NULL, or if the queue is empty. 0 if @data
 * was set with the oldest item available in @queue.
 */
int queue_dequeue(queue_t queue, void **data)
{
    /* Check for problems with null passes and if the queue is empty */
    if(queue == NULL || data == NULL || queue->num_of_nodes == 0)
    {
        return ERROR_FOUND;
    }

    /* Store data from the node being dequeued */
    queue_node *node_to_dequeue = queue->first_in_queue;
    *data = node_to_dequeue->data_in_node;

    /* Reassign the front of the queue to the next in line */
    queue->first_in_queue = node_to_dequeue->next_in_queue;

    queue->num_of_nodes -= ONE_NODE;

    return NO_ERROR;
}

/*
 * queue_delete - Delete data item
 * @queue: Queue in which to delete item
 * @data: Data to delete
 *
 * Find in queue @queue, the first (ie oldest) item equal to @data and delete
 * this item.
 *
 * Return: -1 if @queue or @data are NULL, of if @data was not found in the
 * queue. 0 if @data was found and deleted from @queue.
 */
int queue_delete(queue_t queue, void *data)
{
    /* @queue or @data are NULL */
    if(queue == NULL || data == NULL || queue->first_in_queue == NULL)
        return ERROR_FOUND;

    queue_node *index_node = queue->first_in_queue;

    /* Iterating to find @data */
    while(index_node != NULL)
    {
        if(index_node->data_in_node != data) 
        {
            index_node = index_node->next_in_queue;
            continue;
        }

        else if(index_node->data_in_node == data)
        {
            /* First node in the queue */
            if(index_node->next_in_queue != NULL && index_node->prev_in_queue == NULL)
            {
                queue_node *new_front_node = index_node->next_in_queue;
                new_front_node->prev_in_queue = NULL;

                queue->first_in_queue = new_front_node;
            }

            /* Middle node in the queue */
            else if(index_node->next_in_queue != NULL && index_node->prev_in_queue != NULL)
            {
                queue_node *temp_for_next_node = index_node->next_in_queue;
                queue_node *temp_for_prev_node = index_node->prev_in_queue;

                /* Cut out the index node from the queue with the below two operations 
                   Next node in the queue's previous node is now index's previous node */
                temp_for_next_node->prev_in_queue = index_node->prev_in_queue;

                /* Prev node in the queue's next node is now index's next node */
                temp_for_prev_node->next_in_queue = index_node->next_in_queue;
            }

            /* Last node in the queue */
            else if(index_node->next_in_queue == NULL && index_node->prev_in_queue != NULL)
            {
                queue_node *new_last_node = index_node->prev_in_queue;
                new_last_node->next_in_queue = NULL;

                queue->last_in_queue = new_last_node;
            }

            /* Only node exists in the queue */
            else
            {
                queue->first_in_queue = NULL;
                queue->last_in_queue = NULL;
            }

            /* @data was found so we always remove a node and now we can return */
            queue->num_of_nodes -= ONE_NODE;
            return NO_ERROR;
        }
    }

    /* @data was not found */
    return ERROR_FOUND;
}

/*
 * queue_iterate - Iterate through a queue
 * @queue: Queue to iterate through
 * @func: Function to call on each queue item
 *
 * This function iterates through the items in the queue @queue, from the oldest
 * item to the newest item, and calls the given callback function @func on each
 * item. The callback function receives the current data item as parameter.
 *
 * Note that this function should be resistant to data items being deleted
 * as part of the iteration (ie in @func).
*
 * Return: -1 if @queue or @func are NULL, 0 otherwise.
 */
int queue_iterate(queue_t queue, queue_func_t func)
{
    if(queue == NULL || func == NULL)
        return ERROR_FOUND;

    queue_node *index_node = queue->first_in_queue;

    while(index_node != NULL)
    {
        func(index_node->data_in_node);

        // Interruption Protection
        if(index_node->next_in_queue != NULL)
            index_node = index_node->next_in_queue;
        else
            break;
    }

    return NO_ERROR;
}

/*
 * queue_length - Queue length
 * @queue: Queue to get the length of
 *
 * Return the length of queue @queue.
 *
 * Return: -1 if @queue is NULL. Length of @queue otherwise.
 */
int queue_length(queue_t queue)
{
    if(queue == NULL)
        return ERROR_FOUND;
    else
        return queue->num_of_nodes;
}
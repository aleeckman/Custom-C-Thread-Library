#  ECS 150 Project #2 Report
Andrew Eeckman (914834317) & Wing Cheong Lo (917055287)

## Queue API Implementation

### Queue Data Structure
The queue data structure used in this project takes inspiration from the one
described here on tutorialspoint.com, the link to the exact page can be
found below:
https://www.tutorialspoint.com/data_structures_algorithms/dsa_queue.htm

The queue consists of two data structures:

- ```typedef struct queue``` handles the queue as whole, and keeps track of
three arguments. ```queue_node *first_in_queue``` points to the node at the
front of the queue. As dequeue is called, this pointer is updated to match
the new front of the queue.```queue_node *last_in_queue``` points to the 
node in the back of the queue. As enqueue is called, this pointer is updated
to match the node enqueued. ```int num_of_nodes``` is an integer value meant
to keep track of the total number of nodes in the queue. Calls to dequeue,
delete, and exit will lower this value. Calls to enqueue will increase this 
value.

- ```typedef struct queue_node``` handles each particular node in a queue,
keeping track of three arguments unique to each node. ```void* data_in_node```
points to the memory location where the node's data is stored. Here we
utilize a void pointer as the data stored in each node is not necessarily
guaranteed to be of one type. ```struct queue_node *next_in_queue``` points
to the node that follows the current node. This node was enqueued after
the current one. By keeping track of each node's next node, we have a
chain we can follow to traverse the queue in the direction front to back.
This is NULL for the node in the front of the queue as nothing exists
ahead of it. ```struct queue_node *prev_in_queue```points to the node that 
precedes the current one. This node was enqueued before the current node
and will be dequeued first. By keeping track of each node's previous node,
we form a chain through the queue that we can traverse from back to front.

### Queue Functionality
When ```queue_create()``` is called a new queue is initialized with its first
and last nodes both being set to NULL and its count set to 0. Upon enqueuing,
nodes are added into the queue from the back. The first node added becomes the
node pointed to by ```queue_node *first_in_queue``` and each subsequent node is
added behind it. The last one added is always ```queue_node *last_in_queue```.
When dequeue is called, the node that is first in the queue is removed from the
queue and its next node is made the new front of the queue. The dequeued node's
data can then be accessed as used by the function that called dequeue in the
first place. If it is necessary to delete a particular node from the queue,
users can do so by using the ```queue_delete(...)``` function. To use, simply
pass to it the data you would like to delete from the queue and then the
function will iterate through said queue and delete the node with the matching
data. When the queue is no longer needed, ```queue_destroy(...)``` can 
be called and it will be freed from memory. 

### Queue Testing
To test our Queue API we used the class queue_tester.c. We test all the queue
functions on one global set of data. This is primarily done so that we can 
ensure that all our queue functions work together without error. Using 
separate data sets to test individual functions could have led to 
a potential situation in which errors exist between functions that remain
hidden to us. We wanted to avoid such a situation.

### Queue Limitations
One of the largest problems we encountered while making this project was
the result of using a void pointer to store the data of each node. While
both a necessary and incredibly useful feature, it made it particularly
hard to debug any problems encountered in both phase 2 and phase 1. This
is largely due to the fact that what is stored is an address and not a
particular value. Thus, when attempting to check what is actually inside
of queue, it becomes incredibly difficult as no values are visible.


## UThread Implementation

### UThread Data Structure
The user thread data structure consists of four arguments. The first
argument is ```int tid```, this stores the thread's identification
number. This number is tied to the number of threads in use, kept track
of by the variable ```int num_of_threads```. When a thread is created,
this value goes up and thus the new tid is always 1 more than the thread
created before it ensuring uniqueness. When a thread is destroyed this
number decreases and so the next thread takes the newly freed tid. The
second argument is ```unsigned state```, this keeps track of the state
the thread is in whether or not it is currently running, ready, or blocked. 
The third argument is ```void* stack``` which will store the thread's stack
in memory. The fourth argument is ```uthread_ctx_t ctx``` which is responsible
for storing all of the necessary information about the thread's current
context. Basically, this variable will store the information unique to
a specific thread allowing us to do context switches done the line.

### UThread Functionality
Calling ```uthread_start(...)``` begins the multi-threading process. This
function does two things. The first is that it initializes the "idle thread"
which does not consist of neither an external call or any arguments.
This thread is simply responsible for executing all other threads in the
ready queue and deleting both the ready queue and blocked queue when all
others threads have finished their execution. The second thing this function
does is create the first "true" thread (with an external function call) and
add that to the ready queue. Threads that complete their execution are 
destroyed in ```uthread_exit(...)``` and the function then passes the CPU
to the next thread in the ready queue. Any new thread can be later created
using ```uthread_create(...)``` which will initialize the new thread and
create both a stack and context for it. These new threads are then added
to the ready queue for later execution. At any time during their execution
a thread can call ```uthread_yield(...)``` to pass the CPU to the next thread
in the ready queue, assigning itself to the back in the process. Threads that
attempt to access semaphore resources that are no longer available are placed
in a blocked queue using ```uthread_block()``` until that resource becomes
available again. When that resource is again available, ```uthread_unblock()```
iterates through the blocked queue to find the thread that originally tried
to take such resource and adds that thread back into the ready_queue.

### UThread Testing
The user thread library is tested using the testing classes provided, both
uthread_hello.c and uthread_yield.c. These are primarily used to the functions
start, create, exit, and yield. Both block and unblock are tested later with 
with the implementation of semaphores. 

### UThread Limitations
Other than being difficult to implement, we did not find too many limitations
regarding the user thread library. One in particular, we would like to 
highlight however is the use of ```unsigned state```. While this variable
does help the programmer keep track of where each thread is at in its
execution, it is ultimately unnecessary for the program's logic as
other conditions and variables can be used to indicate a thread's state 
(i.e. if a process is in the ready queue then it is ready).


## Semaphore Implementation

### Semaphore Data Structure
The semaphore data structure is implemented using three arguments. The first,
```size_t resources_avail``` keeps track of the number of resources one
semaphore manages. This value can be any positive number including 0. The
second, ```queue_t blocked_threads``` keeps track of the threads blocked
by this particular semaphore. It is important to differentiate between this
queue and the blocked_queue in uthread.c as the blocked_queue in uthread.c
keeps track of all blocked threads regardless of the semaphore they were
trying to access. The last argument is ```int num_of_blocked_threads```
which merely keeps track of the number of threads in a semaphore's blocked
queue.

### Semaphore Functionality
When a semaphore is created using ```sem_create()``` it is initialized with
a specific count and it's blocked queue is created using ```queue_create()```.
```sem_up(...)``` adds back to this count and unblocks any previously blocked
threads that were blocked when attempting to use this semaphore, if any exist.
To do so it dequeues from its blocked queue and calls ```uthread_unblock()```.
```sem_down(...)``` first checks if there are any available resources left.
If there are, it merely reduces the resource count and returns. If no 
resources are left, it adds the calling thread into its own blocked queue
and calls ```uthread_block(...)``` to block the thread in uthread.c. When 
the semaphore is of no more use it is freed in ```sem_destroy()```.

### Semaphore Testing
Semaphores are tested using sem_simple.c, sem_buffer.c, sem_count.c, 
sem_prime.c as well as a few of our own test cases. To start with sem_simple.c,
We simply hoped to see that basic sem_up and sem_down features were functional.
Other test cases were then used to ensure functionality under more complex
settings.

### Limitations
While sem_prime.c we noticed that our semaphores would eventually freeze up
(i.e. the process would stop, usually around 419). Our implementation seems 
to suffer during large scale tests. Also starvation is not necessarily corrected
for, but we hope that preemption's round-robin model will prevent this.

## Preemption Implementation

### Preemption Functionality
When ```preempt_start()``` is called, we utilize two variables to start the
process: ```struct itimerval timer``` and ```struct sigaction signal_handler```. 
The signal_handler is used to handle signals of the type SIGVTALRM. When such
a signal is encountered, the signal_handler calls upon the linked function:
response_handler, which then calls uthread_yield as we want to give the CPU
to the thread next in line. The timer then sets the frequencies by which
these signals are sent. ```preempt_enable()``` is capable of unblocking
said signals and allowing for the signal_handler to respond appropriately.
```preempt_disable()``` does the opposite and blocks said signals, not
allowing the signal handler to then interrupt the running process. Finally,
```preempt_stop()``` sets the timer interval to zero to reset it and sets
the signal handler to the standard SIG_IGN to ignore it.

### Preeemption Testing
To test preemption, we initialize two threads, thread1 and thread2, with
thread1 being responsible for the creation of thread2. When thread1 starts,
it issues a print statement to notify the user that they are currently in
thread1. It then creates thread2 with ```thread_create(...)```. Immediately
afterwards it enters a while loop. This while loop will execute until a
boolean value that can only be changed by thread2 is set to true. If
preemption works, thread2 should be able to grab hold of the CPU and change
this boolean value so that thread1 can exit the while loop, notifying the user 
with a print statement. It does so and  thus we are confident preemption works. 

### Preeemption Limitations
Not necessarily a limitation, but due to a lack of knowledge of about
implementation, there are likely several lines of unnecessary code in preempt.c.
If given the chance to modify this code again in the future, we would like to
be able to better implement this part of the project using fewer lines.
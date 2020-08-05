/**** DO NOT MODIFY THIS FILE ****/

//User level threads library
// INCLUDES
#include "threads.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// DEFINES
#define SECOND 1000000
#define MAX_NO_OF_THREADS 100 /* in any state */
#define STACK_SIZE 10000 //4096
#define TIME_QUANTUM 1

// Black box code
////////////////////////////////////////////////////////////////////////
#ifdef __x86_64__
// code for 64 bit Intel arch

//typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

//A translation required when using an address of a variable
//Use this as a black box in your code.
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
                 : "=g"(ret)
                 : "0"(addr));
    return ret;
}

#else
// code for 32 bit Intel arch

//typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

//A translation required when using an address of a variable
//Use this as a black box in your code.
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
                 : "=g"(ret)
                 : "0"(addr));
    return ret;
}

#endif
////////////////////////////////////////////////////////////////////////

// Global variables
thread_queue_t* thread_list; /* the list of all threads */
thread_queue_t* ready_list; /* the list of all ready threads */
thread_t* current; /* the current running thread */
int next_thread = 0; /* Used for assigning IDs */
int scheduling_type; /* Scheduling type 0 = FCFS, 1 = RR, 2 = vLOT, 3 = mLOT */
int clean = 0; /* if in cleanup, exit out of dispatch */
unsigned start_time = 0; /* Time a thread is started */

extern void thread_enqueue(thread_t*, thread_queue_t*);
// enqueue to back of queue

extern thread_t* scheduler();
//implementation of thread scheduler, RR and vLOT and mLOT

int CreateThread(void (*f)(void), int weight)
{
    // Return -1 if fail
    if (thread_list->size + 1 > MAX_NO_OF_THREADS) {
        printf("Too many threads\n");
        return -1;
    }
    thread_t* thread = malloc(sizeof(thread_t));

    thread->status = malloc(sizeof(status_t));
    thread->stack = malloc(STACK_SIZE);
    thread->status->id = next_thread;
    thread->status->no_of_bursts = 0;
    thread->status->no_of_continuous_bursts = 0;
    thread->status->total_exec_time = 0;
    thread->status->total_sleep_time = 0;
    thread->status->total_wait_time = 0;
    thread->status->avg_exec_time = 0;
    thread->status->avg_wait_time = 0;
    thread->status->avg_continuous_exec_time = 0;
    thread->status->wake_time = 0;
    thread->weight = weight;
    next_thread++;
    thread->status->state = READY;

    thread->sp = (address_t)thread->stack + STACK_SIZE - sizeof(address_t);
    thread->pc = (address_t)f;
    sigsetjmp(thread->jbuf, 1);
    (thread->jbuf->__jmpbuf)[JB_SP] = translate_address(thread->sp);
    (thread->jbuf->__jmpbuf)[JB_PC] = translate_address(thread->pc);
    sigemptyset(&thread->jbuf->__saved_mask);

    thread_enqueue(thread, ready_list);
    thread_enqueue(thread, thread_list);

    return thread->status->id;
}

// Signal handler
void Dispatch(int sig)
{
    if (clean == 1) {
        return;
    }
    unsigned time_delta = GetCurrentTime() - start_time;
    status_t* s = current->status;
    s->total_exec_time += time_delta;
    // Save state of current thread
    BeginContextSwitch();
    int ret = sigsetjmp(current->jbuf, 1);
    if (ret == 1) {
        return;
    }
    // Iterate through all threads list and deal with them.
    thread_node_t* node = thread_list->head;
    while (node != NULL) {
        switch (node->thread->status->state) {
            // Do nothing
        case READY:
            break;
        case RUNNING:
            break;
        case SUSPENDED:
            break;
        case SLEEPING:
            if (GetCurrentTime() >= node->thread->status->wake_time) {
                thread_enqueue(node->thread, ready_list);
                node->thread->status->state = READY;
            }
            break;
        case FINISHED:
            RemoveFromList(node->thread->status->id, ready_list);
            break;
        }
        node = node->next;
    }
    // Iterate through ready_queue to update wait times
    thread_node_t* ready = ready_list->head;
    while (ready != NULL) {
        if (ready->thread->status->state == READY) {
            // only add wait time for ready threads (in case something else gets in here)
            ready->thread->status->total_wait_time += time_delta;
        }

        ready = ready->next;
    }
    // Get the new scheduled thread
    thread_t* prev = current;
    thread_t* next = GetNextThread();
    current = next;
    RemoveFromList(next->status->id, ready_list); // remove from ready list if still in it
    status_t* stat = next->status;
    stat->state = RUNNING;
    stat->no_of_bursts++;
    // If the same thread was chosen, then this is a continuous burst
    if (prev != current) {
        stat->no_of_continuous_bursts++;
    }
    // Update all thread statistics, because some of them may have changed
    thread_node_t* thread_node = thread_list->head;
    while (thread_node != NULL) {
        status_t* stat = thread_node->thread->status;
        if (stat->no_of_bursts) {
            stat->avg_exec_time = (stat->total_exec_time / stat->no_of_bursts);
            stat->avg_continuous_exec_time = (stat->total_exec_time / stat->no_of_continuous_bursts);
        }
        stat->avg_wait_time = (stat->total_wait_time / (stat->no_of_bursts + 1)); // + 1 b/c num_wait = num_run + 1
        thread_node = thread_node->next;
    }
    start_time = GetCurrentTime();
    start_timer();
    EndContextSwitch();
    siglongjmp(next->jbuf, 1);
}

void Go()
{
    thread_t* t = GetNextThread();
    current = t;
    RemoveFromList(current->status->id, ready_list); // remove from ready list if still there

    t->status->state = RUNNING;
    t->status->no_of_bursts++;
    t->status->no_of_continuous_bursts++;
    start_timer();
    start_time = GetCurrentTime();
    siglongjmp(t->jbuf, 1);
    while (1)
        ;
}

int GetMyId()
{
    return current->status->id;
}

thread_t* GetThread(int thread_id)
{
    if (thread_list->head == NULL) {
        return NULL;
    }
    thread_node_t* node = thread_list->head;
    while (node != NULL && node->thread->status->id != thread_id) {
        node = node->next;
    }
    if (node == NULL) {
        return NULL;
    }
    return node->thread;
}

int DeleteThread(int thread_id)
{
    int currentId = GetMyId();
    thread_t* t = GetThread(thread_id);
    if (t == NULL) {
        return -1;
    }
    t->status->state = FINISHED;
    if (thread_id == currentId) {
        YieldCPU();
    }

    return 0;
}

int RemoveFromList(int thread_id, thread_queue_t* q)
{
    thread_node_t* node = q->head;
    thread_node_t* prev_node = NULL;

    // Find node with corresponding thread_id
    if (!node) {
        return -1;
    }
    while (node->next != NULL && node->thread->status->id != thread_id) {
        prev_node = node;
        node = node->next;
    }
    // If it is not found return -1
    if (node->thread->status->id != thread_id) {
        return -1;
    }
    // Head
    if (node == q->head) {
        if (node->next == NULL) {
            // If the tail and the head are the same thing
            q->tail = NULL;
        }
        // If the head is to be deleted.
        q->head = node->next; 

    }
    // Tail
    else if (node->next == NULL) {
        prev_node->next = NULL;
        q->tail = prev_node;
    }
    // Otherwise in middle / end of list
    else {
        prev_node->next = node->next;
    }
    free(node);
    (q->size)--;
    return 0;
}

// Inserts the thread t at the head of the queue q
void InsertAtHead(thread_t* t, thread_queue_t* q)
{
    thread_node_t* node = malloc(sizeof(thread_node_t));
    node->thread = t;
    node->next = q->head;
    q->head = node;
    if (q->tail == NULL)
        q->tail = node;
    q->size++;
}

// Inserts the thread t at the tail of the queue q
void thread_enqueue(thread_t* t, thread_queue_t* q)
{
    thread_node_t* node = malloc(sizeof(thread_node_t));
    node->thread = t;
    node->next = NULL;

    if (q->tail != NULL)
        q->tail->next = node;
    else
        q->head = node;

    q->tail = node;
    q->size++;
}

// Pops the head of the queue q and returns it
thread_t* thread_dequeue(thread_queue_t* q)
{
    thread_node_t* head_old;
    thread_t* thread;
    head_old = q->head;
    if (!head_old)
        return NULL;
    q->head = head_old->next;
    if (!q->head)
        q->tail = NULL;
    thread = head_old->thread;
    free(head_old);
    q->size--;
    return thread;
}

void YieldCPU()
{
    raise(SIGVTALRM);
}

int GetStatus(int thread_id, status_t* status)
{
    thread_t* t = GetThread(thread_id);
    if (t == NULL)
        return -1;
    status->id = t->status->id;
    status->state = t->status->state;
    status->no_of_bursts = t->status->no_of_bursts;
    status->no_of_continuous_bursts = t->status->no_of_continuous_bursts;
    status->total_exec_time = t->status->total_exec_time;
    status->total_sleep_time = t->status->total_sleep_time;
    status->total_wait_time = t->status->total_wait_time;
    status->avg_exec_time = t->status->avg_exec_time;
    status->avg_wait_time = t->status->avg_wait_time;
    status->avg_continuous_exec_time = t->status->avg_continuous_exec_time;
    return t->status->id;
}

int SuspendThread(int thread_id)
{
    thread_t* t = GetThread(thread_id);
    if (t == NULL)
        return -1;

    t->status->state = SUSPENDED;
    RemoveFromList(thread_id, ready_list);
    if (current->status->id == thread_id)
        YieldCPU();

    return thread_id;
}

int ResumeThread(int thread_id)
{
    thread_t* t = GetThread(thread_id);
    if (t == NULL)
        return -1;

    if (t->status->state != SUSPENDED)
        return thread_id;

    thread_enqueue(t, ready_list);
    t->status->state = READY;
    return thread_id;
}

unsigned GetCurrentTime()
{
    clock_t time = clock();
    unsigned millis = ((double)(time) / CLOCKS_PER_SEC) * 1000;
    return millis;
}

void SleepThread(int sec)
{
    current->status->state = SLEEPING;
    current->status->wake_time = (GetCurrentTime() + (sec * 1000));
    current->status->total_sleep_time += (sec * 1000);
    RemoveFromList(current->status->id, ready_list);
    YieldCPU();
}

void start_timer()
{
    struct itimerval tv;
    tv.it_value.tv_sec = TIME_QUANTUM; //time of first timer
    tv.it_value.tv_usec = 0; //time of first timer
    tv.it_interval.tv_sec = 0; //time of all timers but the first one
    tv.it_interval.tv_usec = 0; //time of all timers but the first one
    setitimer(ITIMER_VIRTUAL, &tv, NULL);
}

//Get the head of thread queue
thread_t* GetNextThread()
{
    return scheduler();
}

/*Print the status of all threads and kill all threads.
In other words, capture the last status of all threads.*/

void CleanUp()
{
    clean = 1;
    // Print contents of status struct
    thread_node_t* node = thread_list->head;
    while (NULL != node) {
        thread_t* t = node->thread;
        status_t* s = t->status;
        printf("thread %d info:\n", s->id);
        switch (s->state) {
        case (RUNNING):
            printf("thread state = RUNNING\n");
            break;
        case (READY):
            printf("thread state = READY\n");
            break;
        case (SLEEPING):
            printf("thread state = SLEEPING\n");
            break;
        case (SUSPENDED):
            printf("thread state = SUSPENDED\n");
            break;
        case (FINISHED):
            printf("thread state = FINISHED\n");
            break;
        }
        printf("num_runs = %d\ntotal_exec_time = %d\n"
               "total_sleep_time = %d\ntotal_wait_time = %d\navg_exec_time = %d\navg_wait_time = %d\n"
               "num_continuous_runs = %d\navg_continuous_exec_time = %d\n\n",
            s->no_of_bursts, s->total_exec_time, s->total_sleep_time, s->total_wait_time, s->avg_exec_time, s->avg_wait_time, s->no_of_continuous_bursts, s->avg_continuous_exec_time);
        node = node->next;
    }
    // delete all threads
    // free up readyQ
    node = ready_list->head;
    thread_node_t* next;
    while (NULL != node) {
        next = node->next;
        RemoveFromList(node->thread->status->id, ready_list);
        node = next;
    }
    free(ready_list);
    //free up threads
    node = thread_list->head;
    while (NULL != node) {
        next = node->next;
        thread_t* t = node->thread;
        RemoveFromList(node->thread->status->id, thread_list);
        free(t->stack);
        free(t->status);
        free(t);
        node = next;
    }
    free(thread_list);
    // exit
    exit(0);
}

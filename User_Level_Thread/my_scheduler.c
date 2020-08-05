/**** You may MODIFY the designated portions of this file ****/
/**** You may also add any additional functions you need for your implementation ****/

/**** Please fill in your (and your partner's) information (Name and PSU email) below and in the variables below
 * Name:
 * Email:
 * Name:
 * Email:
 ****/
#include "threads.h"
#include <time.h>
#include <stdlib.h>
#define MAX_CONTEXT_SWITCH 1000
#define TIME_QUANTUM 1

// Fill out your PSU ID (xyz1234) here
// Make sure to NOT USE your PSU number
char ID_1[] = "";
char ID_2[] = "";

// If you need to have extra variables from threads.c, add them here as `extern`
extern int scheduling_type;
extern thread_queue_t* ready_list;
extern thread_queue_t* thread_list;
extern thread_t* current;
struct timespec begin;
struct timespec end;
long long int context_switch_time[MAX_CONTEXT_SWITCH];
int context_switch_num;

// This function is called once by the test file, before the thread are created
// Feel free to modify this function as much as you want, to initialize variables or structs for instance
void setup(int schedule)
{
    printf("Student IDs:\nID_1: %s\nID_2: %s\n", ID_1, ID_2);
    srand(time(NULL));
    scheduling_type = schedule; // FCFS == 0, RR == 1, vLOT == 2, mLOT == 3
    ready_list = malloc(sizeof(thread_queue_t));
    ready_list->head = ready_list->tail = NULL;
    ready_list->size = 0;

    thread_list = malloc(sizeof(thread_queue_t));
    thread_list->head = ready_list->tail = NULL;
    thread_list->size = 0;

    current = NULL;

    signal(SIGVTALRM, Dispatch);
}

// This function adds the thread t to the ready queue q according the scheduler used
// Feel free to modify this function as much as you want
void InsertWrapper(thread_t* t, thread_queue_t* q)
{
    InsertAtHead(t, q);
}

// This function is called whenever a context switch is starting
void BeginContextSwitch()
{
    clock_gettime(CLOCK_MONOTONIC, &begin);
}

// This function is called whenever a context switch is done
void EndContextSwitch()
{
    clock_gettime(CLOCK_MONOTONIC, &end);
    context_switch_time[context_switch_num] = (long long)
            (((long long)end.tv_nsec - (long long)begin.tv_nsec) / 1000);
    context_switch_num++;
}

// Add extra functions you want here

// Manages the thread scheduling when called
// Returns the tne running thread
thread_t* scheduler()
{
    /**** You may also add variable declarations here, or in the sections below ****/

    double range_value = 0;
    double total_weight = 0;
    thread_node_t* node = malloc(sizeof(thread_node_t*));
    node = ready_list->head;
    /**** End variable declarations ****/
    srand(time(0));
    double random_value = (double)rand() / (double)((unsigned)RAND_MAX + 1);
    /**** NOTE: the last running thread will be at in the `current` variable ****/
    // Make sure we still have some work to do
    if (ready_list->size == 0 && current == NULL)
        return NULL;
    // Random a value x = [0,1]
    // value means the range of the random number
    switch (scheduling_type) {
    case RR: // Round Robin
        /**** Implement the Round Robin scheduler here ****/
        // Your code here
        //current = thread_dequeue(ready_list);
        if(node == NULL){
            printf("%s\n", "Ready_list == NUL");
            return current;
        }
        current = node->thread;
        thread_enqueue(current, ready_list);
        return current;
        /**** End Round Robin code ****/

    case vLOT:// Vanilla Lottery
        /**** Implement the Vanilla Lottery scheduler here ****/
        // Your code here
      ;//this is a empty statement

        //Before we choose from the ready list, we must insert
        //the current thread first
        if(current != NULL){
            if(current->status->state == RUNNING){
                current->status->state = READY;
                thread_enqueue(current, ready_list);
            }
        }
		// Add up all of the weights in the ready_list
		while(node != NULL){
			total_weight += node->thread->weight;
			node = node->next;
		}
		node = ready_list->head;

        //Randomly choose a thread by their weight from the ready list
		while(node != NULL){
			range_value += node-> thread -> weight / total_weight;
			if(random_value <= range_value){
				return node-> thread;
			}
			node = node->next;
		}
        return ready_list->tail->thread;
        /**** End Vanilla Lottery code ****/

    case mLOT: // Modified Lottery
        /**** Implement the Modified Lottery scheduler here ****/
        // Your code here
        // Each node has the same probability, so we can simply 
        // divide one by number of Nodes in the ready_list
      ;//this is a empty statement;
        
        //If the current thread's continuous running quantam <= 
        // current's weights
        if(current != NULL){
            if(current->status->no_of_bursts % (unsigned)current->weight
                    == 0){
                ;
            }
            else if(current->status->no_of_bursts % (unsigned)current->weight < 
                  (unsigned)current->weight) {
                //If the current thread has not spend enough time yet
                return current;
            }
            if(current->status->state == RUNNING){
                current->status->state = READY;
                thread_enqueue(current, ready_list);
            }
        }

		// Add up all of the weights in the ready_list
		while(node != NULL){
			range_value += 1 / (double)ready_list->size;
			if(random_value <= range_value){
				return node->thread;
			}
			node = node->next;
		}
        return ready_list->tail->thread;
        /**** End Modified Lottery code ****/


    case FCFS: // First Come, First Served
        /**** DO NOT MODIFY ****/
        if (current == NULL) {
            // We did not previously have a running thread
            return ready_list->head->thread;
        } else {
            // We had a running thread...
            if (current->status->state == FINISHED || current->status->state == SLEEPING || current->status->state == SUSPENDED) {
                // ... that cannot run anymore
                if (ready_list->size == 0) {
                    return NULL;
                } else {
                    return ready_list->head->thread;
                }
            } else {
                // ... that can still run
                return current;
            }
        }
    default:
        return NULL;
    }
}

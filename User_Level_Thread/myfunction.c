//#include "myfunction.h"
#include <time.h>
#include <stdio.h>
#include "threads.h"
#include <unistd.h>
#define NUM_THREADS 2

extern int arr[];
extern thread_queue_t* ready_list;
extern long long int context_switch_time[];
extern int context_switch_num;

void user_clean_up_thread()
{
    long long int sum = 0;
    while (1) {
        if (ready_list->head->thread->status->id == GetMyId()
                && ready_list->head == ready_list->tail) {
              for (int i = 0; i < NUM_THREADS; i++){
                    printf("arr[%d] = %d \n", i, arr[i]);
                    sum += arr[i];
                }
            printf("Counter = %lld\n\n", sum);
            printf("Number of context switches: %d\n", context_switch_num);
            for(int i = 0; i < context_switch_num ; i++){
                printf("context switch[%d]'s overhead:  %lldms\n", i, context_switch_time[i]);
            }
            CleanUp();
        }
    }
}

void *user_thread_counter()
{
    int id;
    id = GetMyId();  

    struct timespec begin;
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    long long int rf = 0;
    while (1)
    {
        rf++;
        if(rf % 10000000 == 0)
           printf("thread_id: %d    %lld\n", id, rf);

        clock_gettime(CLOCK_MONOTONIC, &current);
        if (((long long)current.tv_sec - (long long)begin.tv_sec) >= 10)
        {
            arr[id] = rf;
            printf("%d\n", arr[id]);
            break;
        }
    }
        DeleteThread(id);
}

void *user_thread_sleeping()
{
    int id;
    id = GetMyId();

    struct timespec begin;
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    long long int rf = 0;
    printf("%d\n", id);
    if (id % 2 == 0)
    {
        sleep(1);
    }
    else
    {
        sleep(1);
    }
    while (1)
    {
        rf++;
        if(rf % 10000000 == 0)
           printf("thread_id: %d    %lld\n", id, rf);      

        clock_gettime(CLOCK_MONOTONIC, &current);
        if (((long long)current.tv_sec - (long long)begin.tv_sec) >= 10)
        {
            arr[id] = rf;
            printf("%d\n", arr[id]);
            break;
        }
    }
    DeleteThread(id);
}


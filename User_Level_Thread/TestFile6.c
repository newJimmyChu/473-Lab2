#include "myfunction.c"
//#include "threads.h"
#include <string.h>
#define NUM_THREADS 2

//Global variable arr

int arr[100];

int main(int argc, char const *argv[])
{
  //thread_t *threads[NUM_THREADS];
  int rc;
  long t;
  void *tab[] = {user_thread_counter, user_thread_sleeping};
  int flag = -1;

  setup(RR);
  //arr = (int *)malloc(100 * sizeof(int));
  long long int sum = 0;

  for (int i = 0; i < 100; i++)
    arr[i] = 0;

  if (argc < 2)
  {
    printf("Usage: %s [counter] | [sleeping]\n", argv[0]);
    return -1;
  }

  flag = !(strcmp(argv[1], "counter")) ? 0 : 1;

  printf("Code under execution .. \n");
  for (t = 0; t < NUM_THREADS  ; t++)
  {
    if(flag == 0)
        CreateThread((void*)user_thread_counter, 1);
    else
        CreateThread((void*)user_thread_sleeping, 1);
  }
  //Here I create another thread to do the clean_up() function
  //after all of the threads finished
  CreateThread(user_clean_up_thread, 1);
  Go();


  return 0;
}

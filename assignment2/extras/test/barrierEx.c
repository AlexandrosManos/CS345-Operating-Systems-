#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_OF_THREADS 8

int res[NUM_OF_THREADS];
int status[NUM_OF_THREADS] = {1, 1, 1, 1, 1, 1, 1, 1};
pthread_barrier_t diceBarrier;

void *roll_dice(void *ptr)
{
    int value = (rand() % 6) + 1;
    int *result = malloc(sizeof(int));
    int index = *(int *)ptr;
    *result = value;
    printf("Thread[%d]:<%d>\n", index, value);
    res[index] = value;
    pthread_barrier_wait(&diceBarrier);
    for (int i = 0; i < NUM_OF_THREADS; i++)
    {
        if (res[index] < res[i])
        {
            status[index] = 0;
        }
    }
    if (status[index] == 1)
    {
        printf("Thread[%d] rolled <%d> and won \n", index, res[index]);
    }
    else
    {
        printf("Thread[%d] rolled <%d> and LOST \n", index, res[index]);
    }
    free(ptr);
}

int main(int argc, char *argv[])
{
    pthread_t t1[NUM_OF_THREADS];
    int rc, i;
    srand(time(NULL));
    pthread_barrier_init(&diceBarrier, NULL, NUM_OF_THREADS);
    for (i = 0; i < NUM_OF_THREADS; i++)
    {
        int *a = malloc(sizeof(int));
        *a = i;
        rc = pthread_create(&t1[i], NULL, &roll_dice, a);
        if (rc)
        {
            printf("Something went wrong\n");
            return -1;
        }
    }

    for (i = 0; i < NUM_OF_THREADS; i++)
    {
        // wait for the thread to finish
        if (pthread_join(t1[i], NULL))
        {
            printf("Something went wrong\n");
            return -1;
        }
    }
    pthread_barrier_destroy(&diceBarrier);
    return 0;
}

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

pthread_mutex_t mutex;
int primes[10] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};

void *roll_dice(void *ptr)
{
    // sleep(1);
    int index = *(int *)ptr;
    printf("<%d> ", primes[index]);
    free(ptr);
}
int main(int argc, char *argv[])
{
    pthread_t t1[10];
    int rc, i;
    // pthread_mutex_init(&mutex, NULL);
    for (i = 0; i < 10; i++)
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

    for (i = 0; i < 10; i++)
    {
        // wait for the thread to finish
        if (pthread_join(t1[i], NULL))
        {
            printf("Something went wrong\n");
            return -1;
        }
    }
    printf("\n");

    // pthread_mutex_destroy(&mutex);
    return 0;
}

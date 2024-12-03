#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_OF_THREADS 4

sem_t semaphore;

void *routine(void *ptr)
{
    sem_wait(&semaphore);
    sleep(1);
    printf("Hello from thread<%d> \n", *(int *)ptr);
    sem_post(&semaphore);
    free(ptr);
}

int main(int argc, char *argv[])
{
    pthread_t t1[NUM_OF_THREADS];
    int rc, i;
    sem_init(&semaphore, 0, 3);
    for (i = 0; i < NUM_OF_THREADS; i++)
    {
        int *a = malloc(sizeof(int));
        *a = i;
        rc = pthread_create(&t1[i], NULL, &routine, a);
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
    sem_destroy(&semaphore);
    return 0;
}

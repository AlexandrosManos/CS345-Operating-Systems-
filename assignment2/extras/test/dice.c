#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

pthread_mutex_t mutex;

void *roll_dice(void *ptr)
{
    int value = (rand() % 6) + 1;
    int *result = malloc(sizeof(int));
    *result = value;
    printf("<%d>\n", value);
    return (void *)result;
}
int main(int argc, char *argv[])
{
    pthread_t t1;
    int rc;
    int *res;
    srand(time(NULL));
    // pthread_mutex_init(&mutex, NULL);
    rc = pthread_create(&t1, NULL, &roll_dice, NULL);
    if (rc)
    {
        printf("Something went wrong\n");
        return -1;
    }
    pthread_exit(0);
    // wait for the thread to finish
    if (pthread_join(t1, (void **)&res))
    {
        printf("Something went wrong\n");
        return -1;
    }
    // pthread_mutex_destroy(&mutex);
    printf("Reuslt: %d\n", *res);
    free(res);
    return 0;
}

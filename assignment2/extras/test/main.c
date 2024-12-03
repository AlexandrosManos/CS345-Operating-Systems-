#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int mails = 0;
pthread_mutex_t mutex;

void *routnie(void *ptr)
{
    for (int i = 0; i < 1000000; i++)
    {
        pthread_mutex_lock(&mutex);
        mails++;
        pthread_mutex_unlock(&mutex);
    }
}
int main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int rc, rc2;
    pthread_mutex_init(&mutex, NULL);
    rc = pthread_create(&t1, NULL, &routnie, NULL);
    if (rc)
    {
        printf("Something went wrong\n");
        return -1;
    }

    rc2 = pthread_create(&t2, NULL, &routnie, NULL);
    if (rc2)
    {
        printf("Something went wrong\n");
        return -1;
    }
    printf("Before Thread\n");
    sleep(1);
    // wait for the thread to finish
    if (pthread_join(t1, NULL))
    {
        printf("Something went wrong\n");
        return -1;
    }

    if (pthread_join(t2, NULL))
    {
        printf("Something went wrong\n");
        return -1;
    }
    pthread_mutex_destroy(&mutex);
    printf("After Thread\n");
    printf("Number of mails: %d\n", mails);
    return 0;
}

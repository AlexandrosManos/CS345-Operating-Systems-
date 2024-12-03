#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

pthread_mutex_t mutexFuel;
pthread_cond_t condFuel;
int fuel = 0;

void *fuel_filling(void *arg)
{
    for (int i = 0; i < 5; i++)
    {
        pthread_mutex_lock(&mutexFuel);
        fuel += 40;
        printf("Filled fuel... %d\n", fuel);
        pthread_mutex_unlock(&mutexFuel);
        pthread_cond_broadcast(&condFuel);
        sleep(1);
    }
}

void *car(void *arg)
{
    int index = *(int *)arg;
    pthread_mutex_lock(&mutexFuel);
    while (fuel < 40)
    {
        printf("Theard[%d]: No fuel. Waiting...\n", index);
        pthread_cond_wait(&condFuel, &mutexFuel);
        // Equivalent to:
        // pthread_mutex_unlock(&mutexFuel);
        // wait for signal on condFuel
        // pthread_mutex_lock(&mutexFuel);
    }
    fuel -= 40;
    printf("Theard[%d]: Got fuel. Now left: %d\n", index, fuel);
    pthread_mutex_unlock(&mutexFuel);
    free(arg);
}

int main(int argc, char *argv[])
{
    pthread_t th[6];
    pthread_mutex_init(&mutexFuel, NULL);
    pthread_cond_init(&condFuel, NULL);
    for (int i = 0; i < 6; i++)
    {
        if (i == 1 || i == 5)
        {
            if (pthread_create(&th[i], NULL, &fuel_filling, NULL) != 0)
            {
                perror("Failed to create thread");
            }
        }
        else
        {
            int *a = malloc(sizeof(int));
            *a = i;
            if (pthread_create(&th[i], NULL, &car, a) != 0)
            {
                perror("Failed to create thread");
            }
        }
    }

    for (int i = 0; i < 6; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            perror("Failed to join thread");
        }
    }
    pthread_mutex_destroy(&mutexFuel);
    pthread_cond_destroy(&condFuel);
    return 0;
}
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <assert.h>

#define T 10
#define N 12
#define NUM_OF_DEP 4
#define BUS_WAITING_TIME 3
#define MIN_STUDENT_NUMBER 4
#define MAX_STUDENT_NUMBER 200

enum department
{
    Math = 1,
    Physics = 2,
    Chemistry = 3,
    Csd = 4
};

pthread_mutex_t mutex;
pthread_mutex_t lockA;
pthread_mutex_t lockB;
pthread_mutex_t UniLock;
pthread_cond_t condstopB;
pthread_cond_t condstopA;
pthread_barrier_t BusBarrier;
sem_t boarding2;
sem_t boarding[NUM_OF_DEP][2];

/*Stores the id and the department of students */
int stopA[MAX_STUDENT_NUMBER][2];
int busArray[MAX_STUDENT_NUMBER][2];
int Uni[MAX_STUDENT_NUMBER][2];
int stopB[MAX_STUDENT_NUMBER][2];
/*An index for each stop and for the bus */
int stopIndex[4] = {0, 0, 0, 0};
/*Stores the number of passenger of each department */
int busDepCounter[NUM_OF_DEP] = {0, 0, 0, 0};

int count[4] = {0, 0, 0, 0};
/* Total number of students */
int students = 0;
int BusStatus = 2;

void printEnum(enum department d);
void init_array(int array[][2], int count);
void print_status();
void print_student(int id, enum department d);
void add_to_stop(int id, enum department d, int stop);
void remove_from_stop(int id, enum department d, int stop);

void *student_routine(void *args)
{
    int id = *(int *)args + 1, i;
    enum department dep = rand() % 4 + 1;
    int study = rand() % 11 + 5;
    /* Create section */
    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("created.\n\n");
    add_to_stop(id, dep, 0);
    print_status();
    pthread_mutex_unlock(&mutex);

    /* Waiting for the bus */
    // pthread_barrier_wait(&BusBarrier);

    /* Boarding from stop A section */
    // sem_wait(&boarding[dep - 1][0]);
    pthread_mutex_lock(&lockA);
    while (BusStatus != 0)
    {
        pthread_cond_wait(&condstopA, &lockA);
    }
    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("boarded to the bus.\n\n");
    remove_from_stop(id, dep, 0);
    add_to_stop(id, dep, 1);
    print_status();
    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&lockA);

    sleep(3);
    /* Stop B section */
    pthread_mutex_lock(&lockB);
    while (BusStatus != 1)
    {
        pthread_cond_wait(&condstopB, &lockB);
    }
    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("got off the bus.\n\n");
    remove_from_stop(id, dep, 1);
    add_to_stop(id, dep, 3);
    print_status();
    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&lockB);

    /* University section */

    usleep(50000);

    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("went to University.\n");
    remove_from_stop(id, dep, 3);
    add_to_stop(id, dep, 2);
    print_status();
    pthread_mutex_unlock(&mutex);
    sleep(study);

    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("studied for %d seconds, and now is heading to stop B.\n\n", study);
    remove_from_stop(id, dep, 2);
    add_to_stop(id, dep, 3);
    print_status();
    pthread_mutex_unlock(&mutex);

    sleep(3);
    /* Stop B section */
    pthread_mutex_lock(&lockB);
    while (BusStatus != 1)
    {
        pthread_cond_wait(&condstopB, &lockB);
    }
    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("boarded to the bus.\n\n");
    remove_from_stop(id, dep, 3);
    add_to_stop(id, dep, 1);
    print_status();
    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&lockB);

    sleep(1);
    /* Home section */
    pthread_mutex_lock(&lockA);
    while (BusStatus != 0)
    {
        pthread_cond_wait(&condstopA, &lockA);
    }
    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("went home.\n\n");
    remove_from_stop(id, dep, 1);
    students--;
    sem_post(&boarding[dep - 1][0]);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&lockA);

    free(args);
    return NULL;
}

void *bus_routine(void *args)
{
    int go = 1, i, j;
    init_array(busArray, N);

    // pthread_barrier_wait(&BusBarrier);
    printf("\033[0;32mBording time\n\n\033[0;37m");
    // for (i = 0; i < NUM_OF_DEP; i++)
    // {
    //     for (j = 0; j < N / 4 + 1; j++)
    //     {
    //         sem_post(&boarding[i][0]);
    //     }
    // }
    sleep(4);

    while (students > 0)
    {
        BusStatus = 0;
        if (count[0] > 0)
        {
            printf("Broadcasting...\n");
            pthread_cond_broadcast(&condstopA);
        }
        else
        {
            printf("Found none\n");
        }
        sleep(3);
        printf("After slepp\n");
        pthread_mutex_lock(&mutex);
        printf("\033[0;31mBus is on the way to University...!\033[0;37m\n");
        BusStatus = 2;
        sleep(T);
        printf("Bus arrived at University!\n\n");
        pthread_mutex_unlock(&mutex);

        BusStatus = 1;
        pthread_cond_broadcast(&condstopB);
        sleep(3);
        pthread_mutex_lock(&mutex);
        printf("\033[0;31mBus is heading to Stop A\033[0;37m\n\n");
        BusStatus = 2;
        sleep(T);
        printf("Bus arrived to Stop A\n\n");
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int i, rc;
    pthread_t *StudentsThr;
    pthread_t busThr;
    /*----Initialize--------*/
    srand(time(NULL));
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&UniLock, NULL);

    sem_init(&boarding2, 0, 0);
    // sem_init(&boarding, 0, N);
    for (i = 0; i < NUM_OF_DEP; i++)
    {
        sem_init(&boarding[i][0], 0, 0);
        sem_init(&boarding[i][1], 0, 0);
    }

    pthread_mutex_init(&lockA, NULL);
    pthread_cond_init(&condstopA, NULL);
    pthread_mutex_init(&lockB, NULL);
    pthread_cond_init(&condstopB, NULL);

    printf("Enter the number of student:\n");
    scanf("%d", &students);
    while (students < MIN_STUDENT_NUMBER || students > MAX_STUDENT_NUMBER)
    {
        printf("The number of student must be at least %d and less than %d.\n", MIN_STUDENT_NUMBER, MAX_STUDENT_NUMBER);
        scanf("%d", &students);
    }
    printf("\n");
    /*Initialize arrays with zeros*/
    init_array(stopA, MAX_STUDENT_NUMBER);
    init_array(stopB, MAX_STUDENT_NUMBER);
    init_array(Uni, MAX_STUDENT_NUMBER);
    pthread_barrier_init(&BusBarrier, NULL, students + 1);
    StudentsThr = malloc(students * sizeof(pthread_t));
    for (i = 0; i < students; i++)
    {
        int *a = malloc(sizeof(int));
        *a = i;
        rc = pthread_create(&StudentsThr[i], NULL, &student_routine, a);
        usleep(50000);
        if (rc)
        {
            printf("Failed to create thread[%d]\n", i);
            return -1;
        }
    }
    rc = pthread_create(&busThr, NULL, &bus_routine, NULL);
    if (rc)
    {
        printf("Failed to create bus thread\n");
        return -1;
    }
    for (i = 0; i < students; i++)
    {
        // wait for the thread to finish
        if (pthread_join(StudentsThr[i], NULL))
        {
            printf("Failed to join thread[%d]\n", i);
            return -1;
        }
    }
    if (pthread_join(busThr, NULL))
    {
        printf("Failed to join bus thread\n");
        return -1;
    }
    free(StudentsThr);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&UniLock);
    // sem_destroy(&boarding);
    sem_destroy(&boarding2);
    pthread_barrier_destroy(&BusBarrier);
    pthread_mutex_destroy(&lockA);
    pthread_cond_destroy(&condstopA);
    pthread_mutex_destroy(&lockB);
    pthread_cond_destroy(&condstopB);

    return 0;
}

void printEnum(enum department d)
{
    switch (d)
    {
    case 1:
        printf("Math");
        break;
    case 2:
        printf("Physics");
        break;
    case 3:
        printf("Chemistry");
        break;
    case 4:
        printf("Csd");
        break;
    default:
        assert(0);
    }
    return;
}

int boarding_check(enum department d)
{
    if (busDepCounter[d - 1] >= N / 4)
    {
        return 0;
    }
    return 1;
}

void init_array(int array[][2], int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        array[i][0] = 0;
        array[i][1] = 0;
    }
    return;
}

void print_status()
{
    int i;
    printf("\nStop A: ");
    for (i = 0; i < stopIndex[0]; i++)
    {
        if (stopA[i][0] != 0 && stopA[i][1] != 0)
        {
            printf("[%d, ", stopA[i][0]);
            printEnum(stopA[i][1]);
            printf("] ");
        }
    }
    printf("\nBus: ");
    for (i = 0; i < stopIndex[1]; i++)
    {
        if (busArray[i][0] != 0 && busArray[i][1] != 0)
        {
            printf("[%d, ", busArray[i][0]);
            printEnum(busArray[i][1]);
            printf("] ");
        }
    }
    printf("\nUniversity: ");
    for (i = 0; i < stopIndex[2]; i++)
    {
        if (Uni[i][0] != 0 && Uni[i][1] != 0)
        {
            printf("[%d, ", Uni[i][0]);
            printEnum(Uni[i][1]);
            printf("] ");
        }
    }
    printf("\nStop B: ");
    for (i = 0; i < stopIndex[3]; i++)
    {
        if (stopB[i][0] != 0 && stopB[i][1] != 0)
        {
            printf("[%d, ", stopB[i][0]);
            printEnum(stopB[i][1]);
            printf("] ");
        }
    }
    printf("\n\n");
}

void print_student(int id, enum department d)
{
    if (id == 0)
    {
        return;
    }

    printf("Student %d (", id);
    printEnum(d);
    printf(") ");
    return;
}
void add_to_stop(int id, enum department d, int stop)
{
    int i;
    i = stopIndex[stop];
    switch (stop)
    {
    case 0:
        stopA[i][0] = id;
        stopA[i][1] = d;
        break;
    case 1:
        busArray[i][0] = id;
        busArray[i][1] = d;
        busDepCounter[d - 1]++;
        break;
    case 2:
        Uni[i][0] = id;
        Uni[i][1] = d;
        break;
    case 3:
        stopB[i][0] = id;
        stopB[i][1] = d;
        break;
    default:
        assert(0);
        break;
    }
    count[stop]++;
    stopIndex[stop]++;
    return;
}
void remove_from_stop(int id, enum department d, int stop)
{
    int i;

    switch (stop)
    {
    case 0:
        for (i = 0; i < stopIndex[stop]; i++)
        {
            if (stopA[i][0] == id && stopA[i][1] == d)
            {
                stopA[i][0] = 0;
                stopA[i][1] = 0;
                break;
            }
        }
        break;
    case 1:
        for (i = 0; i < stopIndex[stop]; i++)
        {
            if (busArray[i][0] == id && busArray[i][1] == d)
            {
                busArray[i][0] = 0;
                busArray[i][1] = 0;
                busDepCounter[d - 1]--;
                break;
            }
        }

        break;
    case 2:
        for (i = 0; i < stopIndex[stop]; i++)
        {
            if (Uni[i][0] == id && Uni[i][1] == d)
            {
                Uni[i][0] = 0;
                Uni[i][1] = 0;
                break;
            }
        }
        break;
    case 3:
        for (i = 0; i < stopIndex[stop]; i++)
        {
            if (stopB[i][0] == id && stopB[i][1] == d)
            {
                stopB[i][0] = 0;
                stopB[i][1] = 0;
                break;
            }
        }
        break;
    default:
        assert(0);
        break;
    }
    count[stop]--;
    return;
}

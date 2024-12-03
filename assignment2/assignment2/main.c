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
#define MIN_STUDENT_NUMBER 13
#define MAX_STUDENT_NUMBER 200

/*Enum for department*/
enum department
{
    Math = 1,
    Physics = 2,
    Chemistry = 3,
    Csd = 4
};

/*A lock for the prints mostly and boarding/drop off*/
pthread_mutex_t mutex;

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
/**
 * Bus Status equal to 0, if the bus is waiting on stop A
 * Bus Status equal to 1, if the bus arrived at University
 * Bus Status equal to 2, if the bus is on the road
 * Bus Status equal to 4, if the bus is waiting on University
 * Bus Status equal to 5, if the bus is dropping off the student from University to stop A
 */
int BusStatus = 2;

/* Prints ------------------------*/
void printEnum(enum department d);
void init_array(int array[][2], int count);
void print_status();
void print_student(int id, enum department d);
/*---------------------------------------------*/
/*Check if the student with id, should wait for his turn*/
int st_turn(int id, enum department d, int stop);
/* Checks if there is enough space for a student from d department*/
int can_board(enum department d);
/*Add a student to stop */
void add_to_stop(int id, enum department d, int stop);
/*Remove a student from stop */
void remove_from_stop(int id, enum department d, int stop);

/* Return 1 if bus should wait*/
int wait_bus();

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

    /* Boarding from stop A section */
    do
    {
        while (BusStatus != 0)
        {
            usleep(500);
        }
        while (st_turn(id, dep, 0) != 1)
        {
            usleep(50000);
        }

        if (can_board(dep) == 0)
        {
            pthread_mutex_lock(&mutex);
            print_student(id, dep);
            printf("cannot enter the Bus\n");
            pthread_mutex_unlock(&mutex);
            sleep(2 * T);
        }

    } while (pthread_mutex_trylock(&mutex) != 0);
    /* In case someone board with from the same department at the same time */
    print_student(id, dep);
    printf("boarded to the bus.\n\n");
    remove_from_stop(id, dep, 0);
    add_to_stop(id, dep, 1);
    print_status();
    pthread_mutex_unlock(&mutex);
    /*Until the bus arrives to University*/
    sleep(T);
    /* Stop B section */
    while (BusStatus != 1)
    {
        usleep(5000);
    }
    while (st_turn(id, dep, 3) != 1)
    {
        usleep(50000);
    }
    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("got off the bus.\n\n");
    remove_from_stop(id, dep, 1);
    add_to_stop(id, dep, 3);
    print_status();
    pthread_mutex_unlock(&mutex);

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
    /* Stop B to home section */
    while (BusStatus != 4)
    {
        usleep(5000);
    }
    while (st_turn(id, dep, 3) != 1)
    {
        usleep(500);
    }
    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("boarded to the bus.\n\n");
    remove_from_stop(id, dep, 3);
    add_to_stop(id, dep, 1);
    print_status();
    pthread_mutex_unlock(&mutex);

    /* Home section */
    /*Sleep until the bus returns from University*/
    sleep(T);
    while (BusStatus != 5)
    {
        usleep(500);
    }
    while (st_turn(id, dep, 3) != 1)
    {
        usleep(500);
    }
    pthread_mutex_lock(&mutex);
    print_student(id, dep);
    printf("went home.\n\n");
    remove_from_stop(id, dep, 1);
    students--;
    pthread_mutex_unlock(&mutex);

    free(args);
    return NULL;
}

void *bus_routine(void *args)
{
    int go = 1, i, j;
    init_array(busArray, N);

    sleep(1);
    /* Just a green print to help me track the output */
    printf("\033[0;32mFirst Bording\n\n\033[0;37m");
    while (students > 0)
    {
        /*Boarding from stop A, wait until none of the student can board*/
        BusStatus = 0;
        while (wait_bus() == 1)
        {
            sleep(BUS_WAITING_TIME);
        }

        if (students == 0)
        {
            break;
        }
        pthread_mutex_lock(&mutex);
        printf("\033[0;31mBus is on the way to University...!\033[0;37m\n");
        BusStatus = 2;
        pthread_mutex_unlock(&mutex);
        sleep(T);

        pthread_mutex_lock(&mutex);
        printf("Bus arrived at University!\n\n");
        pthread_mutex_unlock(&mutex);

        BusStatus = 1;
        /*Wait until the bus is empty*/
        while (count[1] > 0)
        {
            sleep(BUS_WAITING_TIME);
        }

        BusStatus = 4;
        /*Wait for the student to board from stop B*/
        while (wait_bus() == 1)
        {
            sleep(BUS_WAITING_TIME);
        }
        pthread_mutex_lock(&mutex);
        printf("\033[0;35mBus is heading to Stop A\033[0;37m\n\n");
        BusStatus = 2;
        sleep(T);
        printf("Bus arrived to Stop A\n\n");
        pthread_mutex_unlock(&mutex);
        BusStatus = 5;
        /*Wait until the bus is empty*/
        while (count[1] > 0)
        {
            sleep(BUS_WAITING_TIME);
        }
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
    // wait for the thread to finish
    if (pthread_join(busThr, NULL))
    {
        printf("Failed to join bus thread\n");
        return -1;
    }
    free(StudentsThr);
    pthread_mutex_destroy(&mutex);
    return 0;
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

/*Add a student to stop */
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
/*Remove a student from stop */
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
/*Check if the student with id, should wait for his turn*/
int st_turn(int id, enum department d, int stop)
{
    int i;
    /* Stop A */
    if (stop == 0)
    {
        for (i = 0; i < stopIndex[0]; i++)
        {
            /* If there is a student from a department that can board to the bus,
             before the student with the given id */
            if (can_board(stopA[i][1]) == 1 && stopA[i][0] != id)
            {
                return 0;
            }
            /* If there is no other student before the student with this id, its his turn*/
            if (stopA[i][0] == id)
            {
                break;
            }
        }
        return 1;
    } /* Waiting for the student to get off the bus */
    else if (stop == 3 && (BusStatus == 1 || BusStatus == 5))
    {
        for (i = 0; i < stopIndex[1]; i++)
        {
            if (busArray[i][0] != 0 && busArray[i][0] != id)
            {
                return 0;
            }
            if (busArray[i][0] == id)
            {
                break;
            }
        }
        return 1;
    } /* The bus arrived at University and waiting for the student on stop B to board */
    else if (stop == 3 && BusStatus == 4)
    {
        for (i = 0; i < stopIndex[3]; i++)
        {
            if (stopB[i][0] != 0 && stopB[i][0] != id)
            {
                return 0;
            }
            if (stopB[i][0] == id)
            {
                break;
            }
        }
        return 1;
    }

    assert(0);
}
/* Checks if there is enough space for a student from d department*/
int can_board(enum department d)
{
    /* Removed student */
    if (d == 0)
    {
        return 0;
    }
    /* N/4 students on the bus from the same department */
    if (busDepCounter[d - 1] == N / 4)
    {
        return 0;
    }
    return 1;
}

/* Return 1 if bus should wait*/
int wait_bus()
{
    int i;
    if (BusStatus == 0)
    {
        for (i = 0; i < stopIndex[BusStatus]; i++)
        {
            if (can_board(stopA[i][1]) == 1)
            {
                return 1;
            }
        }
    }
    else if (BusStatus == 4)
    {
        for (i = 0; i < stopIndex[3]; i++)
        {
            if (can_board(stopB[i][1]) == 1)
            {
                return 1;
            }
        }
    }
    return 0;
}

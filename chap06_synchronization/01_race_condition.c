/*
 * Chapter 6: Synchronization Tools
 * Demo 01: Race condition
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    THREAD_COUNT = 4,
    LOOP_COUNT = 50000
};

static int counter = 0;

static void *worker(void *arg)
{
    (void)arg;

    for (int i = 0; i < LOOP_COUNT; i++) {
        counter++;
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&threads[i], NULL, worker, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }

    printf("Expected = %d\n", THREAD_COUNT * LOOP_COUNT);
    printf("Actual   = %d\n", counter);
    return 0;
}

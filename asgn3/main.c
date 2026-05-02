#include "dine.h"
#include "display.h"

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct {
    int id;
    int cycles;
} phil_parameters;

sem_t semaphores[NUM_PHILOSOPHERS]; //[fork1, f2, ...]
sem_t sem_lock; //semaphore that locks threads for printing
int holds_fork[NUM_PHILOSOPHERS][NUM_PHILOSOPHERS];
char *state[NUM_PHILOSOPHERS];

void dawdle() {
/*
* sleep for a random amount of time between 0 and DAWDLEFACTOR
* milliseconds. This routine is somewhat unreliable, since it
* doesn’t take into account the possiblity that the nanosleep
* could be interrupted for some legitimate reason.
*/
    struct timespec tv;
    int msec = (int)((((double)random()) / RAND_MAX) * DAWDLEFACTOR);

    tv.tv_sec = 0;
    tv.tv_nsec = 1000000 * msec;
    if (nanosleep(&tv,NULL) == -1) {
        perror("nanosleep");
    }
}

void* start(void* ptr){
    phil_parameters *params = (phil_parameters*)ptr;
    int thread_id = params->id;
    int cycles = params->cycles;

    state[thread_id] = "";
    display_status();
    
    for(int c = 0; c < cycles; c++){
        //even philosophers and odd philosophers
        if(thread_id % 2 == 0){
            sem_wait(&semaphores[(thread_id + 1) % NUM_PHILOSOPHERS]); //right first
            holds_fork[thread_id][(thread_id + 1) % NUM_PHILOSOPHERS] = 1;
            display_status();

            sem_wait(&semaphores[thread_id]); //then left
            holds_fork[thread_id][thread_id] = 1;
            display_status();

        } else {
            sem_wait(&semaphores[thread_id]); //left first
            holds_fork[thread_id][thread_id] = 1;
            display_status();

            sem_wait(&semaphores[(thread_id + 1) % NUM_PHILOSOPHERS]); //then right
            holds_fork[thread_id][(thread_id + 1) % NUM_PHILOSOPHERS] = 1;
            display_status();
        }

        // --- eat ---
        state[thread_id] = "Eat";
        display_status();
        dawdle();

        // --- put forks down ---
        state[thread_id] = "";
        sem_post(&semaphores[thread_id]);
        holds_fork[thread_id][thread_id] = 0;
        display_status();

        sem_post(&semaphores[(thread_id + 1) % NUM_PHILOSOPHERS]);
        holds_fork[thread_id][(thread_id + 1) % NUM_PHILOSOPHERS] = 0;
        display_status();

        // --- think ---
        if (c < cycles - 1) {
            state[thread_id] = "Think";
            display_status();
            dawdle();
            
            state[thread_id] = "";
            display_status();
        }
    }
    //--- done ---
    state[thread_id] = "";
    display_status();

    return NULL;
}

int main(int argc, char *argv[]){
    int cycles = 1;
    if(argc > 1){
        cycles = atoi(argv[1]);
    }

    pthread_t threads[NUM_PHILOSOPHERS]; //[philosopher1, p2, ...]
    phil_parameters params[NUM_PHILOSOPHERS]; //[struct1, struct2, ...]

    // initialize state
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        state[i] = "";
    }

    //creating semaphores (forks)
    sem_init(&sem_lock, 0, 1);
    if (sem_init(&sem_lock, 0, 1) == -1) {
        printf("sem_init FAILED");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        sem_init(&semaphores[i], 0, 1);

        if (sem_init(&semaphores[i], 0, 1) == -1) {
            printf("sem_init for a fork FAILED");
            exit(EXIT_FAILURE);
        }
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec + tv.tv_usec);

    print_header();

    //creating threads (philosophores)
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        params[i].id = i;
        params[i].cycles = cycles;
        pthread_create(&threads[i], NULL, start, &params[i]);

        int res = pthread_create(&threads[i], NULL, start, &params[i]);
        if (res != 0) {
            printf("pthread_create FAILED");
            exit(EXIT_FAILURE);
        }
    }

    //waiting for threads to finish
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        pthread_join(threads[i], NULL);
    }

    print_footer();

    return 0;
}
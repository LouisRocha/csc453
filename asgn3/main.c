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

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

#ifndef DAWDLEFACTOR
#define DAWDLEFACTOR 1000
#endif

sem_t semaphores[NUM_PHILOSOPHERS]; //[fork1, f2, ...]
sem_t sem_lock; //semaphore that locks threads for printing

void* start(void* ptr){
    int thread_id = *(int*)ptr;
    char label = 'A' + thread_id;
    printf("Philosopher %c started\n", label);

    //even philosophers and odd philosophers
    if(thread_id % 2 == 0){
        sem_wait(&semaphores[(thread_id + 1) % NUM_PHILOSOPHERS]); //right first
        sem_wait(&semaphores[thread_id]); //then left
    } else {
        sem_wait(&semaphores[thread_id]); //left first
        sem_wait(&semaphores[(thread_id + 1) % NUM_PHILOSOPHERS]); //then left
    }

    return NULL;
}

int main(){
    pthread_t threads[NUM_PHILOSOPHERS]; //[philosopher1, p2, ...]
    int thread_ids[NUM_PHILOSOPHERS]; //[0, 1, 2, ...]

    //creating semaphores (forks)
    sem_init(&sem_lock, 0, 1);

    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        sem_init(&semaphores[i], 0, 1);
    }

    //creating threads (philosophores)
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, start, &thread_ids[i]);
    }

    //waiting for threads to finish
    for(int i = 0; i < NUM_PHILOSOPHERS; i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}
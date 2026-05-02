#include "dine.h"

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


extern sem_t sem_lock;
extern int holds_fork[][NUM_PHILOSOPHERS];  
extern char *state[];

void print_header() {
    // top border
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("|=============");
    }
    printf("|\n");

    // philosopher labels (A, B, C, ...)
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("|      %c      ", 'A' + i);
    }
    printf("|\n");

    // bottom border
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("|=============");
    }
    printf("|\n");
}

void print_footer() {
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("|=============");
    }
    printf("|\n");
}

void display_status() {
    sem_wait(&sem_lock);
    for(int p = 0; p < NUM_PHILOSOPHERS; p++) {
        char forkstring[NUM_PHILOSOPHERS + 1];
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            if(holds_fork[p][i] == 1){
                forkstring[i] = '0' + i;
            } else {
                forkstring[i] = '-';
            }
        }
        forkstring[NUM_PHILOSOPHERS] = '\0';
        char col[NUM_PHILOSOPHERS + 8];
        if (strlen(state[p]) > 0) {
            snprintf(col, sizeof(col), "%s %s", forkstring, state[p]);
        } else {
            snprintf(col, sizeof(col), "%s", forkstring);
        }
        printf("| %-11s ", col);
    }
    printf("|\n");
    sem_post(&sem_lock);  // unlock
}
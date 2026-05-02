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


/* globals variables in main.c */
extern sem_t sem_lock;
extern int holds_fork[][NUM_PHILOSOPHERS];  
extern char *state[];

/*
 * print_header() - prints the top border of output.
 * pretty self explanatory.
 */
void print_header() {
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("|=============");
    }
    printf("|\n");
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("|      %c      ", 'A' + i);
    }
    printf("|\n");
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("|=============");
    }
    printf("|\n");
}

/*
 * print_footer() - prints the bottom border of output.
 * pretty self explanatory.
 */
void print_footer() {
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("|=============");
    }
    printf("|\n");
}

/*
 * display_status() - prints one status row showing every philosopher's
 * current forks held and state they are in. Uses sem_lock to ensure 
 * only one thread prints at a time. Called after every state change.
 */
void display_status() {
    sem_wait(&sem_lock);
    for(int p = 0; p < NUM_PHILOSOPHERS; p++) {
        /* build the forkstring: which forks are held if at all */
        char forkstring[NUM_PHILOSOPHERS + 1];
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            if(holds_fork[p][i] == 1){
                forkstring[i] = '0' + i;
            } else {
                forkstring[i] = '-';
            }
        }
        forkstring[NUM_PHILOSOPHERS] = '\0';

        /* combine forkstring and the state label into one column */
        char col[NUM_PHILOSOPHERS + MAX_STATE_LEN]; 
        if (strlen(state[p]) > 0) {
            snprintf(col, sizeof(col), "%s %s", forkstring, state[p]);
        } else {
            snprintf(col, sizeof(col), "%s", forkstring);
        }
        printf("| %-11s ", col);
    }
    printf("|\n");
    sem_post(&sem_lock);
}
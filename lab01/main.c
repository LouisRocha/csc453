#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


int main(){
    int fd[2];

    if (pipe(fd) == -1){
        printf("pipe unsuccessful\n");
        return 1;
    }

    pid_t pid = fork();
    
    if(pid == 0){
        // child

    } else if(pid > 0){
        // parent...this value is childs pid

        pid_t pid2 = fork();
        if(pid2 == 0){
            // 2nd child
        } else if (pid2 > 0){
            //parent 2... this value is child 2 pid
        } else {
            printf("value < 0");
            perror;
        }
    } else {
        printf("value < 0");
        perror;
    }
    
    return 0;
}
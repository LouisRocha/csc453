#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>

//fd[0] = read
//fd[1] = write

int main(){

	int fd[2];
	if(pipe(fd) == -1){
		perror("pipe error");
		exit(1);
	}

// ls
	pid_t p1 = fork();
	
	if(p1 < 0){
		 perror("fork failed in child 1");
		 exit(1);

	} else if(p1 == 0){
		close(fd[0]);			// close imm. because we don't use it
		dup2(fd[1], STDOUT_FILENO);	// redirect standard output -> pipe
		close(fd[1]);			// close because no longer needed
		execlp("ls", "ls", NULL);	// we are done so we run "ls"
		
		perror("exec() failed");	//this should never run if execlp runs successfully
		exit(1);
	}

//sort -r
	pid_t p2 = fork();
	
	if(p2 < 0){
		perror("fork failed in child 2");
		exit(1);
	
	} else if(p2 == 0){
	
		close(fd[1]);			// close imm. because we don't use it
		dup2(fd[0], STDIN_FILENO);	// redirect standard input -> pipe
		close(fd[0]);			// close because no longer needed
		
		int outfile = open("outfile", O_CREAT | O_WRONLY | O_TRUNC, 0644); //create outfile with permissions + flags
		if (outfile < 0) {
			perror("outfile error");
        		exit(1);
		 }
		
		dup2(outfile, STDOUT_FILENO); 	//redirect
		close(outfile);
		execlp("sort", "sort", "-r", NULL);
		
		perror("exec() failed");
		exit(1);					
	}	
	
	close(fd[0]); //make sure to close both descriptors because we are done with them
	close(fd[1]);

	int status; //for childs exit status
	int status2;

	waitpid(p1, &status, 0); //make sure child processes finish before parent 
	waitpid(p2, &status2, 0);

	if(!WIFEXITED(status)){ //check to make sure the child terminated normally
		exit(1);
	}
	if(!WIFEXITED(status2)){
		exit(1);
	}	
	
	return 0;
}

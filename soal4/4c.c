#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);

void ForkAndLSAndPipeOutput(int *linkout){
	pid_t pid;
	int status;

	pid = fork();
	if(pid < 0){
		die("forkandls");
	}

	if(pid == 0){

		//make the output pipes to linkout
		close(linkout[0]);
		dup2(linkout[1],STDOUT_FILENO);
		close(linkout[1]);

		char * argv[] = {"ls","-1",NULL};
		execv("/usr/bin/ls",argv);
	}else{
		//make the input to linkin
		close(linkout[1]);
		dup2(linkout[0],STDIN_FILENO);
		close(linkout[0]);

		char * argv[] = {"wc","-l", NULL};
		execv("/usr/bin/wc",argv);
		return;
	}
}

void ForkAndHeadAndPipeOutput(int * linkin, int * linkout){
	pid_t pid;
	int status;
	char endout[2] = "\0";

	pid = fork();
	if(pid < 0){
		die("forkandhead");
	}

	if(pid == 0){
		//make the input to linkin
		close(linkin[1]);
		dup2(linkin[0],STDIN_FILENO);
		close(linkin[0]);

		//make the output pipes to linkout
		close(linkout[0]);
		dup2(linkout[1],STDOUT_FILENO);
		close(linkout[1]);

		char * argv[] = {"wc","-l", NULL};
		execv("/usr/bin/wc",argv);
	}else{
		// wait(&status);
		return;
	}
}

void printPipeRead(int * pipe){
	char buf[4096];
	read(pipe[0],buf,4096);
	printf("%s",buf);
}


int main() {
	int link1[2];
	// int link2[2];
	pid_t pid;
	char out[4096];

	if (pipe(link1)<0)
		die("pipe1");
	// if (pipe(link2)<0)
    // 	die("pipe2");

	ForkAndLSAndPipeOutput(link1);
	// ForkAndHeadAndPipeOutput(link1,link2);
	// read(link2[0],out,4096);
	// printf("%s\n",out);
	return 0;
}
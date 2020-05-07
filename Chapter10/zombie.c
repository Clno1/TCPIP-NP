#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>

int main(int argc,char *argv[])
{	
	pid_t pid=fork();
	int status;

	if (pid==0) {
		return 3;
	} else {
		printf("Child 1's PID is %d\n",pid);
		
		pid_t pid2=fork();

		if (pid2==0) {
			exit(7);
		} else {
			printf("Child 2's PID is %d\n",pid2);
			wait(&status);
			if (WIFEXITED(status))
				printf("%d\n",WEXITSTATUS(status));
			
			wait(&status);
			if (WIFEXITED(status))
                                printf("%d\n",WEXITSTATUS(status));
			
			sleep(30);
		}
		
	}
	return 0;
}

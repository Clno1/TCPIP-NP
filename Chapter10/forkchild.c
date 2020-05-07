#include<stdio.h>
#include<unistd.h>

int main(int argc,char *args[])
{
	int before=1;
	int after=1;

	pid_t pid=fork();

	if (pid==0) {
		after++;
		puts("Hi I am a Child process");
		printf("pid:%d before:%d after:%d\n",pid,before,after);
	} else {
		after--;
		printf("Get Child's pid:%d\n",pid);
		printf("before:%d after:%d\n",before,after);
	}
	return 0;
}

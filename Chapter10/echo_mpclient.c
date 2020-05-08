#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *meessge);
void read_routine(int sock,char *buf);
void write_routine(int sock,char *buf);

int main(int argc,char *argv[])
{
	int sock;
	pid_t pid;
	char buf[BUF_SIZE];
	struct sockaddr_in serv_adr;

	if (argc!=3) {
		printf("Usage : %s <IP> <Port>\n",argv[0]);
		exit(1);
	}

	//先connect连接到服务器
	sock=socket(PF_INET,SOCK_STREAM,0);
	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

	if (connect(sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
		error_handling("connect() error!");


	//fork()出子进程，然后父子进程分离
	//子进程做写，父进程做读
	pid=fork();
	if (pid==0) write_routine(sock,buf);
	else read_routine(sock,buf);

	close(sock);
	return 0;
}

void read_routine(int sock,char *buf) {
	while (1) {
		int str_len=read(sock,buf,BUF_SIZE);
		if (str_len==0) return;
		
		buf[str_len]=0;
		printf("Message from server: %s",buf);
	}
}
void write_routine(int sock,char *buf) {
	while (1) {
		fgets(buf,BUF_SIZE,stdin);
		if (!strcmp(buf,"q\n") || !strcmp(buf,"Q\n")) {
			shutdown(sock,SHUT_WR);
			return;
		}
		write(sock,buf,strlen(buf));
	}
}
void error_handling(char *message) {
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

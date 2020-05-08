#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);
void read_childproc(int sig);

int main(int argc,char *argv[])
{
	int serv_sock,clnt_sock;
	struct sockaddr_in serv_adr,clnt_adr;

	pid_t pid;
	struct sigaction act;
	socklen_t adr_sz;
	int str_len,state;
	char buf[BUF_SIZE];
	if (argc!=2) {
		printf("Usage : %s <port>\n",argv[0]);
		exit(1);
	}
	
	//注册处理子进程函数
	act.sa_handler=read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	state=sigaction(SIGCHLD,&act,0);
	

	//socket bind listen 三件套
	serv_sock=socket(PF_INET,SOCK_STREAM,0);
	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));

	if (bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if (listen(serv_sock,5)==-1)
		error_handling("listen() error");

	//
	while (1) {
		adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_adr,&adr_sz);
		if (clnt_sock==-1) continue;
		else puts("new client connected...");
		
		//走到这里证明client连接成功
		pid=fork();
		if (pid==-1) {	//fork 出错了
			close(clnt_sock);
			continue;
		}
		if (pid==0) {	//字进程：
			while ((str_len=read(clnt_sock,buf,BUF_SIZE))!=0)
				write(clnt_sock,buf,str_len);
			puts("client disconnect");
			close(clnt_sock);
			close(serv_sock);
			return 0;	//子进程处理完直接结束，结束前记得把socket都给close掉
		} 
		else close(clnt_sock);	//父进程： close掉client的socket之后继续循环等待下一个client连接
	}
	close(serv_sock);	//这里父进程的server的socket也close了，这样的话父亲和儿子都各自把client/server的socket关闭
	return 0;
}

//处理子进程的函数
void read_childproc(int sig) {
	pid_t pid;
	int status;
	pid=waitpid(-1,&status,WNOHANG);
	printf("remove proc id: %d \n",pid);
}
//处理错误
void error_handling(char *message) {
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

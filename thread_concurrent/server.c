#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<pthread.h>
#include<fcntl.h>

#include"wrap.h"

#define MAXLINE 8192
#define SERV_PORT 8000

struct s_info {		//定义一个结构体, 将地址结构跟cfd捆绑
	struct sockaddr_in cliaddr;
	int connfd;
};

//线程函数,处理客户端请求
void *do_work(void *arg) {
	int i,n;
	struct s_info *ts=(struct s_info*)arg;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];      //#define INET_ADDRSTRLEN 16  可用"[+d"查看

	//正常的处理客户端请求
	while (1) {
		n=Read(ts->connfd,buf,MAXLINE);
		if (n==0) {
			printf("the client %d closed...\n",ts->connfd);
			break;
		}
		printf("received from %s at PORT %d\n",inet_ntop(AF_INET,&(*ts).cliaddr.sin_addr,str,sizeof(str)),ntohs((*ts).cliaddr.sin_port));


		for (i=0;i<n;i++)
			buf[i]=toupper(buf[i]);
		Write(STDOUT_FILENO,buf,n);
		Write(ts->connfd,buf,n);
	}
	Close(ts->connfd);

	return (void *)0;
}

int main(void) 
{
	struct sockaddr_in servaddr,cliaddr;
	socklen_t cliaddr_len;
	int listenfd,connfd;
	pthread_t tid;
	struct s_info ts[256];		//根据最大线程数创建结构体数组.
	int i=0;

	//创建socket
	listenfd=Socket(AF_INET,SOCK_STREAM,0);

	//设置号结构体servaddr然后Bind
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(SERV_PORT);

	Bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	//Listen设置监听上限
	Listen(listenfd,128);
	printf("Accepting client connect...\n");

	//主线程不断监听，并且用子线程去处理
	while (1) {
		//阻塞监听客户端连接
		cliaddr_len=sizeof(cliaddr);
		connfd=Accept(listenfd,(struct sockaddr *)&cliaddr,&cliaddr_len);
		ts[i].cliaddr=cliaddr;
		ts[i].connfd=connfd;

		//创建进程
		pthread_create(&tid,NULL,do_work,(void *)&ts[i]);
		pthread_detach(tid);	//子线程分离，防止僵尸进程

		i++;
	}

	return 0;
}

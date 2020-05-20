#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<errno.h>
#include<ctype.h>

#include"wrap.h"

#define MAXLINE 8192
#define SERV_PORT 8000
#define OPEN_MAX 5000

int main(int argc,char *argv[])
{
	int i,listenfd,connfd,sockfd;
	int n,num=0;
	ssize_t nready,efd,res;
	char buf[MAXLINE],str[INET_ADDRSTRLEN];
	socklen_t clilen;

	struct sockaddr_in cliaddr,servaddr;
	struct epoll_event tep,ep[OPEN_MAX];

	//前面这些东西没什么特别的,socket->bind->listen
	listenfd=Socket(AF_INET,SOCK_STREAM,0);

	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(SERV_PORT);

	Bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));

	Listen(listenfd,20);

	//从这里开始epoll特别的地方
	//epoll_create创建一棵OPEN_MAX节点大小的 红黑树
	efd=epoll_create(OPEN_MAX);
	if (efd==-1)
		perr_exit("epoll_create error");

	//设置listenfd的epoll_event结构体，并把它添加到监听红黑树上
	tep.events=EPOLLIN; 
	tep.data.fd=listenfd;
	res=epoll_ctl(efd,EPOLL_CTL_ADD,listenfd,&tep);
	if (res==-1)
		perr_exit("epoll_wait error");

	//服务器监听过程
	for ( ; ; ) {
		/*epoll_wait()为server阻塞监听事件, ep为struct epoll_event类型数组, OPEN_MAX为数组容量, -1表永久阻塞*/
		nready=epoll_wait(efd,ep,OPEN_MAX,-1);
		if (nready==-1)
			perr_exit("epoll_wait error");

		//从epoll_wait后出来的ep数组就是有反应的事件
		for (i=0;i<nready;i++) {
			if (!(ep[i].events & EPOLLIN))		//不是"读"事件
				continue;
			if (ep[i].data.fd==listenfd) {	//是lfd，是请求连接
				clilen=sizeof(cliaddr);
				connfd=Accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);

				printf("received from %s at PORT %d\n",inet_ntop(AF_INET,&cliaddr.sin_addr,str,sizeof(str)),ntohs(cliaddr.sin_port));
				printf("cfd %d--client %d\n",connfd,++num);

				//Accept之后把新连接的读事件添加到监听红黑树上
				tep.events=EPOLLIN; 
				tep.data.fd=connfd;
				res=epoll_ctl(efd,EPOLL_CTL_ADD,connfd,&tep);
				if (res==-1)
					perr_exit("epoll_ctl_ error");
			} else {	//不是lfd，是读事件
				sockfd=ep[i].data.fd;
				n=Read(sockfd,buf,MAXLINE);

				if (n==0) {	//对端关闭链接，从树上取下
					res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
					if (res==-1)
						perr_exit("epoll ctl error");
					Close(sockfd);
					printf("client[%d] closed connection\n",sockfd);
				} else if (n<0) {	//读错误，取下
					perror("read n<0 error:");
					res=epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
					Close(sockfd);
				} else {	//正常读到数据
					for (i=0;i<n;i++)
						buf[i]=toupper(buf[i]);

					Write(STDOUT_FILENO,buf,n);
					Write(sockfd,buf,n);
				}
			}	
		}

	}
	Close(listenfd);
	Close(efd);

	return 0;
}

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include"wrap.h"

#define MAXLINE 8192
#define SERV_PORT 6666

int main(int argc,char *argv[])
{
	struct sockaddr_in servaddr;
	char buf[MAXLINE];
	int sockfd,n;

	//创建socket
	sockfd=Socket(AF_INET,SOCK_STREAM,0);

	//写好servaddr结构体，connect
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	inet_pton(AF_INET,"127.0.0.1",&servaddr.sin_addr);
	servaddr.sin_port=htons(SERV_PORT);

	Connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	//不断从标准输入，输入后写到服务器，然后从服务器读入写到屏幕
	while (fgets(buf,MAXLINE,stdin)!=NULL) {
		Write(sockfd,buf,strlen(buf));
		n=Read(sockfd,buf,MAXLINE);
		if (n==0) {
			printf("the other side has been closed.\n");
			break;
		} else
			Write(STDOUT_FILENO,buf,n);
	}

	Close(sockfd);
	return 0;
}

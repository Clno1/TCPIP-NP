#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<ctype.h>

#include"wrap.h"

#define SERV_PORT 6666

int main(int argc,char *argv[])
{
	int i,j,n,maxi;
	int nready,client[FD_SETSIZE];		//自定义数组client, 防止遍历1024个文件描述符  FD_SETSIZE默认为1024
	int maxfd,listenfd,connfd,sockfd;
	char buf[BUFSIZ],str[INET_ADDRSTRLEN];	//#define INET_ADDRSTRLEN 16

	struct sockaddr_in clie_addr,serv_addr;
	socklen_t clie_addr_len;
	fd_set rset,allset;	// rset 读事件文件描述符集合 allset用来暂存

	//创建socket
	listenfd=Socket(AF_INET,SOCK_STREAM,0);

	//端口复用
	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	//写好结构体并Bind
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(SERV_PORT);
	Bind(listenfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	//Listen设置监听上限
	Listen(listenfd,128);


	maxfd=listenfd;		//maxfd是轮询fd最大值，起初listenfd为最大

	maxi=-1;	
	for (i=0;i<FD_SETSIZE;i++)
		client[i]=-1;	//初始化client[]
	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);

	while (1) {
		//因为select函数会改变bitmap，所以复制一份然后再去select
		rset=allset;
		nready=select(maxfd+1,&rset,NULL,NULL,NULL);
		if (nready<0)
			perr_exit("select error");

		//看看select结果中有没有listenfd，有的话就是新连接请求
		if (FD_ISSET(listenfd,&rset)) {
			clie_addr_len=sizeof(clie_addr);
			connfd=Accept(listenfd,(struct sockaddr*)&clie_addr,&clie_addr_len);

			printf("received from %s ar PORT %d\n",inet_ntop(AF_INET,&clie_addr.sin_addr,str,sizeof(str)),
			ntohs(clie_addr.sin_port));
			
			for (i=0;i<FD_SETSIZE;i++)	//优化：把刚刚建立连接的socketfd存到client[]数组里，存到从小到达第一个未用位置
				if (client[i]<0) {
					client[i]=connfd;
					break;
				}

			if (i==FD_SETSIZE) {	//连接到达上限，危险
				fputs("too many clients\n",stderr);
				exit(1);
			}

			FD_SET(connfd,&allset);	//更新监听bitmap

			if (connfd>maxfd)
				maxfd=connfd;	//用数组优化过版本，maxfd并不是特别有用了

			if (i>maxi) maxi=i;

			if (--nready==0) continue;	//就监听到一个反应时间还是请求连接，后面不用轮询了
		}

		//client[i]数组里存的都是已经建立连接的socketfd，那么这里轮询，如果这里的fd有反应就是读写请求
		//maxi是一个优化，把fd保存起来，省去每次从0到1024轮询的时间
		for (i=0;i<=maxi;i++) {
			if ((sockfd=client[i])<0)
				continue;
			if (FD_ISSET(sockfd,&rset)) {
				if ((n=Read(sockfd,buf,sizeof(buf)))==0) {	//Read返回0，连接终止
					Close(sockfd);
					FD_CLR(sockfd,&allset);
					client[i]=-1;
				} else if (n>0) {	//从对端读到数据
					for (j=0;j<n;j++)
						buf[j]=toupper(buf[j]);
					Write(sockfd,buf,n);
					Write(STDOUT_FILENO,buf,n);
				}
				if (--nready==0) break;		//处理完所有反应了
			}
		}
	}
	
	Close(listenfd);
	return 0;
}

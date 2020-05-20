#include<stdio.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<sys/wait.h>
#include<ctype.h>
#include<unistd.h>

#include"wrap.h"

//缓冲区大小，端口号
#define MAXLINE 8192
#define SERV_PORT 8000	

//信号，回收子线程函数
void do_sigchild(int num) {
	while (waitpid(0,NULL,WNOHANG)>0);
}

int main(void)
{
	struct sockaddr_in servaddr,cliaddr;
	socklen_t cliaddr_len;
	int listenfd,connfd;

	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];
	int i,n;
	pid_t pid;
	
	//信号
	struct sigaction newact;
	newact.sa_handler=do_sigchild;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags=0;
	sigaction(SIGCHLD,&newact,NULL);

	//Socket生成listenfd
	listenfd=Socket(AF_INET,SOCK_STREAM,0);
	
	//端口复用，写法是写死的
	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	
	//写好结构体开始Bind
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(SERV_PORT);
	Bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	//Listen设置监听上限
	Listen(listenfd,20);
	printf("Accepting connections...\n");


	//主函数,不断监听，🈶一个客户连接就fork()一个子进程处理
	while (1) {
		//Accept阻塞监听客户connfd
		cliaddr_len=sizeof(cliaddr);
		connfd=Accept(listenfd,(struct sockaddr *)&cliaddr,&cliaddr_len);
		
		//到这里的话，肯定有客户连接上来了
		pid=fork();
		if (pid==0) {		//子进程处理客户端请求
			Close(listenfd);

			while (1) {

				//从connfd读入数据进来buf
				n=Read(connfd,buf,MAXLINE);
				if (n==0) {	//Read函数返回值n=0，读入结束
					printf("the other side has been closed.\n");
					break;
				}
				//输出信息出来看一看
				printf("received from %s at PORT %d\n",inet_ntop(AF_INET,&cliaddr.sin_addr,str,sizeof(str)),ntohs(cliaddr.sin_port));
				//大小写转化
				for (i=0;i<n;i++)
					buf[i]=toupper(buf[i]);
				//把读到的信息输出到：屏幕和connfd返回去
				Write(STDOUT_FILENO,buf,n);
				Write(connfd,buf,n);
			}

			Close(connfd);
			return 0;
		} else if (pid>0) {	//父进程继续监听，用不着客户端socket
			Close(connfd);
		} else {		//fork失败，错误处理
			perr_exit("fork");
		}
	}
	
	return 0;
}

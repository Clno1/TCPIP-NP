#include<stdio.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/select.h>
#define BUF_SIZE 30

int main(int argc,char *argv[]) 
{
	fd_set reads,temps;
	int result,str_len;
	char buf[BUF_SIZE];
	struct timeval timeout;
	FD_ZERO(&reads);	//初始化为0
	FD_SET(0,&reads);	//注册文件描述符0（控制台输入）

	while (1)
	{
		temps=reads;	//调用select后除了发生变化的描述符，其他都为0，所以复制一份副本来调用select
		timeout.tv_sec=5;
		timeout.tv_usec=0;
		result=select(1,&temps,0,0,&timeout);

		if (result==-1) {	//错误
			puts("select() error!");
			break;
		}
		else if (result==0) {	//超时
			puts("Time out");
		}
		else {		//返回监听到的文件描述符
			if (FD_ISSET(0,&temps)) {	//是否监听到0了
				str_len=read(0,buf,BUF_SIZE);
				buf[str_len]=0;
				printf("message from console: %s",buf);
			}
		}
	}
	return 0;
}

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc,char *argv[])
{
	int serv_sd,clnt_sd;
	FILE * fp;	//文件描述符
	char buf[BUF_SIZE];
	int read_cnt;

	struct sockaddr_in serv_adr,clnt_adr;
	socklen_t clnt_adr_sz;

	if (argc!=2) {
		printf("Usage: %s <port>\n",argv[0]);
		exit(1);
	}

	fp=fopen("file_server.c","rb");
	//Step 1
	serv_sd=socket(PF_INET,SOCK_STREAM,0);
	//Step 2
	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	bind(serv_sd,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
	//Step 3  没有对错误情况处理
	listen(serv_sd,5);
	//Step 4  没有对错误情况处理
	//accept() return a socket
	clnt_adr_sz=sizeof(clnt_adr);
	clnt_sd=accept(serv_sd,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
	//Step 5
	while (1) {
		read_cnt=fread((void *)buf,1,BUF_SIZE,fp);
		if (read_cnt<BUF_SIZE) {
			write(clnt_sd,buf,read_cnt);
			break;
		}
		write(clnt_sd,buf,BUF_SIZE);
	}
	
	//Attention : shundown
	//只关闭输出流，还可以读入
	shutdown(clnt_sd,SHUT_WR);

	read(clnt_sd,buf,BUF_SIZE);
	printf("MEssage from client: %s \n",buf);

	//Finally : close
	fclose(fp);
	close(clnt_sd); close(serv_sd);
	return 0;
}

void error_handling(char *message) {
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

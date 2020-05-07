#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc,char *argv[])
{
	int serv_sock,clnt_sock;	//server's and client's socket 
	char message[BUF_SIZE];		//Cache
	int str_len,i;

	struct sockaddr_in serv_adr,clnt_adr;	//The struct of bind()
	socklen_t clnt_adr_sz;

	if (argc!=2) {
		printf("Usage : %s <port>\n",argv[0]);
		exit(1);
	}
	
	//Step 1 : create a socket
	serv_sock=socket(PF_INET,SOCK_STREAM,0);  //try to create a socket
	if (serv_sock==-1)	//-1 means failing to create socket  
		error_handling("sockrt() error");

	//Step 2 : try to bind()
	memset(&serv_adr,0,sizeof(serv_adr));	//memset the struct, then to do
	serv_adr.sin_family=AF_INET;	//AF_INET means IP
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);	//get IP address
	serv_adr.sin_port=htons(atoi(argv[1]));		//input to Port

	if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");		//Fail to bind()
	
	//Step 3 : listen to client
	if (listen(serv_sock,5)==-1)
		error_handling("listen() error");
	
	//Step 4 : now client can connect and communicate
	clnt_adr_sz=sizeof(clnt_adr);
	for (int i=0;i<5;i++) {
		clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);	//Try to accept client's connection, accept() will create a socket
		if (clnt_sock==-1) error_handling("accept() error");
		else printf("Connected client %d \n",i+1);

		//Accept sucessfully,now can communicate
		while ((str_len=read(clnt_sock,message,BUF_SIZE))!=0)
			write(clnt_sock,message,str_len);

		//Step 5 : close the socket to close connection
		close(clnt_sock);
	}

	//Finally
	close(serv_sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

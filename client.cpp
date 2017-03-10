#include<cstdio>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/errno.h>
#include<iostream>
#include<fcntl.h>
#include<netdb.h>
#include<cstring>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstdlib>
#include<string>
#include<sstream>
using namespace std;
int main(int argc,char** argv){
	struct sockaddr_in servaddr;
	int sockfd;
	char buffer[256];
	char data[256];
	fd_set allset,rset;
	FD_ZERO(&rset);
	FD_ZERO(&allset);
	//set socker
	if(argc!=3){
		perror("argument error");
		exit(0);
	}
	int port=atoi(argv[2]);
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket error");
		exit(1);
	}
	//cout<<"1"<<endl;
	//set
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(port);
	inet_pton(AF_INET,argv[1],&servaddr.sin_addr.s_addr);
	//cout<<"2"<<endl;
	if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0){
		perror("connect error");
		exit(1);
	}
	cout<<"connect success"<<endl;
	FD_SET(0,&allset);
	FD_SET(sockfd,&allset);
	for(;;){
		bzero(data,256);
		bzero(buffer,256);
		rset=allset;
		if(select(4,&rset,NULL,NULL,NULL)<0){
			perror("select error");
			exit(1);
		}
		if(FD_ISSET(0,&rset)){
			cin.getline(buffer,256);
			string str(buffer);
			stringstream ss;
			ss<<str;
			string s;
			ss>>s;
			if(s=="exit"){
				break;
			}
			if(write(sockfd,buffer,256)<0){
				perror("write error");
			}
		}
		int num;
		if(FD_ISSET(3,&rset)){
			if((num=read(sockfd,&data,255))<0){
				perror("read error");
			}
			else {
				cout<<data;
			}
		}
	}
	close(sockfd);
	return 0;
}

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
#include<sstream>
#include<string>
#include<vector>
#define PORT 5859
using namespace std;
int main(void){
	int sockfd,connfd,listenfd;
	socklen_t len;
	struct sockaddr_in servaddr;
	struct sockaddr_in clieaddr;
	fd_set allset,rset;//one for select one for all
	int maxfd;
	int client[FD_SETSIZE];
	int nready,count,maxi;
	char buffer[256];
	int nbytes;
	char hello[]="[Server] Hello, anonymous! From: ";
	char come[]="[Server] Someone is coming!\n";
	char front[]="[Server] ";
	string namelist[FD_SETSIZE];
	string addrlist[FD_SETSIZE];
	int portlist[FD_SETSIZE];
	char anonymous[]="anonymous";
	char re2[]="[Server] ERROR: ";
	char errorcommand[]="[Server] ERROR: Error command.\n";
	char point[]=".";
	for(int i=0;i<FD_SETSIZE;i++){
		string str(anonymous);
		string s3;
		namelist[i]=str;
		addrlist[i]=s3;
		portlist[i]=0;
	}

	//clear fdset
	FD_ZERO(&rset);
	FD_ZERO(&allset);
	//socket()
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
		cout<<"error to socket";
	//set socket
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(PORT);

	//bind
	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0){
		perror("bind error");
		exit(1);
	};

	//listen
	if(listen(listenfd,20)<0){
		perror("listen failed");
		exit(1);
	}
	maxfd=listenfd;
	maxi=-1;
	for(int i=0;i<FD_SETSIZE;i++) client[i]=-1;
	FD_SET(listenfd,&allset);
	//	cout<<"hi"<<endl;
	//select
	//handle client
	for(;;){
		rset=allset;
		nready=select(maxfd+1,&rset,NULL,NULL,NULL);
		if(nready<0){
			perror("select error");
			exit(1);
		}
		if(FD_ISSET(listenfd,&rset)){

			len=sizeof(clieaddr);
			connfd=accept(listenfd,(struct sockaddr*)&clieaddr,&len);
			if(connfd==-1){
				perror("accept error");
			}
			else{
				char address1[INET_ADDRSTRLEN];
				string newaddr(address1);
				inet_ntop(AF_INET,&(clieaddr.sin_addr),address1,INET_ADDRSTRLEN);
				int portnum=htons(clieaddr.sin_port);
				addrlist[connfd]=newaddr;
				portlist[connfd]=portnum;

			//	cout<<address1<<endl;
				char ch[256];
				sprintf(ch,"%s%s/%d\n",hello,address1,portnum);
				for(count=0;count<FD_SETSIZE;count++){
					if(client[count]<0){
						client[count]=connfd;
						break;
					}
				}
				if(count==FD_SETSIZE){
					perror("too many clients");
					exit(1);
				}
				if(write(connfd,ch,strlen(ch))<0) perror("hello error");
				FD_SET(connfd,&allset);
				if(connfd>maxfd) maxfd=connfd;
				if(count>maxi) maxi=count;
				for(int i=3;i<FD_SETSIZE;i++){
					if(i!=connfd&&i!=listenfd&&FD_ISSET(i,&allset)){
						if(write(i,come,strlen(come))<0) perror("coming error");
					}
				}
				//	cout<<nready<<endl;
			//	if(--nready<=0) continue;
			}
		}
		for(int i=0;i<=maxi;i++){
		//	cout<<i<<" ";
			if((sockfd=client[i])<0) continue;
			if(FD_ISSET(sockfd,&rset)&&sockfd!=listenfd){
				if((nbytes=read(sockfd,&buffer,256))<=0){
					if(nbytes==0){
						cout<<"someone is leaving"<<endl;
						close(sockfd);
						FD_CLR(sockfd,&allset);
						client[i]=-1;
						string temp=namelist[sockfd];
						string sss(anonymous);
						namelist[sockfd]=sss;
						for(int j=3;j<FD_SETSIZE;j++){
							if(j!=listenfd&&j!=sockfd&&FD_ISSET(j,&allset)){
								char leave[256];
								char offline[]="is offline.";
								sprintf(leave,"%s%s %s\n",front,temp.c_str(),offline);
								if(write(j,leave,strlen(leave))<0) perror("leaving error");
							}
						}
					}
					else{
						cout<<"nbytes:"<<nbytes<<endl;
						perror("closed uncorrectly");
						exit(0);
					}
				}
				else{
					string s(buffer);
					cout<<"s:"<<s<<endl;
					vector<string> v;
					stringstream ss;
					ss<<s;
					string input;
					while(ss>>input){
						v.push_back(input);
					}
					if(v.size()==0) continue;
					if(v.size()==1&&((v[0].size()!=3||(v[0].size()==3&&v[0]!="who")))){
						if(write(sockfd,errorcommand,strlen(errorcommand))<0) perror("write error");
						continue;
					}
					//who
					if(v.size()==1&&v[0]=="who"){
						for(int j=3;j<FD_SETSIZE;j++){
							if(j!=listenfd&&FD_ISSET(j,&allset)){
								char ch1[256];
								char isme[]=" ->me";
								if(j==sockfd) sprintf(ch1,"%s%s %s/%d%s\n",front,(namelist[j]).c_str(),addrlist[j].c_str(),portlist[j],isme);
								else sprintf(ch1,"%s%s %s/%d\n",front,(namelist[j]).c_str(),addrlist[j].c_str(),portlist[j]);
								if(write(sockfd,ch1,strlen(ch1))<0) perror("who error");
							}
						}
					}
					//else if(s.size()<5){
				//		if(write(sockfd,errorcommand,strlen(errorcommand))<0) perror("write error");
			//			continue;
		//			}
					else{
                        string ss=s.substr(5,s.size()-5);
						//name
						if(s.substr(0,5)=="name "){
							cout<<"ss:"<<ss<<endl;
							if(ss.size()==0){
								if(write(sockfd,errorcommand,strlen(errorcommand))<0) perror("write error");
								continue;
							}
							if(ss==anonymous){
								char re1[]="[Sever] ERROR: Username cannot be anonymous.\n";
								if(write(sockfd,re1,strlen(re1))<0) perror("write error");
								continue;
							}
							int mark=0;
							for(int j=0;j<FD_SETSIZE;j++){
								if(ss==namelist[j]&&j!=sockfd){
									mark=1;
									char re3[]="has been used by others.";
									char re4[256];
									sprintf(re4,"%s%s %s\n",re2,ss.c_str(),re3);
									if(write(sockfd,re4,strlen(re4))<0) perror("write error");
									break;
								}
							}
							if(mark==1) continue;
							char re5[]="[Server] ERROR: Username can only consists of 2~12 English letters.\n";
							if(ss.size()<2||ss.size()>12){
								if(write(sockfd,re5,strlen(re5))<0) perror("write error");
								continue;
							}
							for(int j=0;j<ss.size();j++){
								if(!((ss[j]>='a'&&ss[j]<='z')||(ss[j]>='A'&&ss[j]<='Z'))){
									if(write(sockfd,re5,strlen(re5))<0) perror("write error");
									mark=1;
									break;
								}
							}
							if(mark==1) continue;
							char re6[]="[Server] You're now known as ";
							char re7[256];
							sprintf(re7,"%s%s%s\n",re6,ss.c_str(),point);
							if(write(sockfd,re7,strlen(re7))<0) perror("write error");
							for(int j=0;j<FD_SETSIZE;j++){
								if(j!=listenfd&&j!=sockfd&&FD_ISSET(j,&allset)){
									char ch2[256];
									char re8[]="is now knowns as ";
									sprintf(ch2,"%s%s %s%s%s\n",front,(namelist[sockfd]).c_str(),re8,ss.c_str(),point);
									if(write(j,ch2,strlen(ch2))<0) perror("who error");
								}
							}
							namelist[sockfd]=ss;
						}
						//tell
                        else if(s.substr(0,5)=="tell "){
                            int k=-1;
                            for(int j=0;j<ss.size();j++){
                                if(ss[j]==' '){
                                    k=j;
                                    break;
                                }
                            }
                            if(k==-1){
                                if(write(sockfd,errorcommand,strlen(errorcommand))<0) perror("write error");
                                continue;
                            }
                            else{
								string receiver=ss.substr(0,k);
                                string message=ss.substr(k+1,ss.size()-k-1);
                                cout<<"receiver:"<<receiver<<endl;
                                cout<<"message:"<<message<<endl;
                                if(message.size()==0){
                                    if(write(sockfd,errorcommand,strlen(errorcommand))<0) perror("write error");
                                    continue;
                                }
                                if(namelist[sockfd]==anonymous){
                              		char re9[]="[Server] ERROR: You are anonymous.\n";
                                	if(write(sockfd,re9,strlen(re9))<0) perror("write error");
                                	continue;
                            	}
								if(receiver=="anonymous"){
                                    char re10[]="[Server] ERROR: The client to which you sent is anonymous.\n";
                                    if(write(sockfd,re10,strlen(re10))<0) perror("write error");
                                    continue;
                                }
                                else{
                                    int l=-1;
                                    for(int j=0;j<FD_SETSIZE;j++){
                                        if(receiver==namelist[j]){
                                            l=j;
                                            break;
                                        }
                                    }
                                    if(l==-1){
                                        char re11[]="[Server] ERROR: The receiver doesn't exist.\n";
                                        if(write(sockfd,re11,strlen(re11))<0) perror("write error");
                                            continue;
                                    }
                                    else{
                                        for(int j=3;j<FD_SETSIZE;j++){
                                            if(j!=listenfd&&FD_ISSET(j,&allset)){
                                                char re12[]="[Server] SUCCESS: Your message has been sent.\n";
                                                if(j==sockfd){
                                                    if(write(sockfd,re12,strlen(re12))<0) perror("write error");
                                                }
                                                if(j==l){
                                                    char ch3[256];
                                                    char re13[]="tell you ";
                                                    sprintf(ch3,"%s%s %s%s\n",front,(namelist[sockfd]).c_str(),re13,message.c_str());
													if(write(j,ch3,strlen(ch3))<0) perror("write error");
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        //yell
                        else if(s.substr(0,5)=="yell "){
                            if(ss.size()==0){
                                if(write(sockfd,errorcommand,strlen(errorcommand))<0) perror("write error");
                                continue;
                            }
                            else{
                                for(int j=3;j<FD_SETSIZE;j++){
                                    if(j!=listenfd&&FD_ISSET(j,&allset)){
                                        char ch4[256];
                                        char re14[]="yell ";
                                        sprintf(ch4,"%s%s %s%s\n",front,(namelist[sockfd]).c_str(),re14,ss.c_str());
                                        if(write(j,ch4,strlen(ch4))<0) perror("write error");
                                    }
                                }
                            }
                        }
                        //else
                        else{
                            if(write(sockfd,errorcommand,strlen(errorcommand))<0) perror("write error");
                            continue;
                        }
					}
				}
				if(--nready<=0) break;
			}
		}
	}
}

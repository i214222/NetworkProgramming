#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include "server.h"

#define SERV_TCP_PORT    6999
#define SERV_HOST_ADDR   "nplinux2.cs.nctu.edu.tw"  /*host addr for server */
#define LISTENQ 5

int newsockfd;
/*
void reaper(int i)
{
	union wait status;
	
	while(wait3(&status, WNOHANG,(struct rusage *)0)>=0);
	
}
*/

int main()
{
	chdir("/u/gcs/104/0456026/ras/");
	setenv("PATH","bin:.",1);
	int sockfd, n;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;
	
	/* Open a TCP socket(an Internet stream socket).*/
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("server: can't open stream socket");
		exit(-2);
	}
	/* Bind our local address so that the client can send to us.*/
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family 	  = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = htons(SERV_TCP_PORT);
	
	int isSetSocketOK = 1;
	// Set the socket to be reusable even if the socket has not been cleaned.
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &isSetSocketOK, sizeof(int) );
	
	if(bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		perror("server: can't bind local address");
	
	listen(sockfd, LISTENQ);
	
	printf("%s\n","Server running...waiting for connections.");
	//(void)signal(SIGCHLD, reaper);
	
	while(1){
		/*
		 * Wait for a connection from a client process.
		 * This is an example of a concurrent server.
		 */
		 
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr,&clilen);
		printf("%s\n","Received request...");
		if(newsockfd < 0)
			perror("server: accept error");
		
		if((childpid = fork()) < 0)
			perror("server: fork error");
		
		else if (childpid == 0){   /*child*/
				close(sockfd); 		
	//			read(sockfd,str,100);
				
				char welcomeMsg[] = "****************************************\n** Welcome to the information server. **\n****************************************\n";
//				send(newsockfd,"noooo\n",7,0);
				send(newsockfd,welcomeMsg,strlen(welcomeMsg),0);
				send(newsockfd,"% ",2,0);
				
				if(linenum == 0){
					Inlinepipe1[0] = -1;
					Inlinepipe1[1] = -1;
					Inlinepipe2[0] = -1;
					Inlinepipe2[1] = -1;
					for(int i = 0 ; i < MaxPipeNum ; i++)
					{
						pipearray[i].pfds[0] = -1;
						pipearray[i].pfds[1] = -1;
						pipearray[i].exist = false; 	
					}
					
				}
				
				char buf[MaxInputChar];
				int inputLen = 0;
				while((n = recv(newsockfd,buf,MaxInputChar,0))>0){
					if(buf[n-1] != '\n'){
						strcat(input,buf);
						inputLen += n;
						memset(buf, 0, sizeof(buf));
						continue;
					}
					strcat(input,buf);
					inputLen += n;
					input[inputLen] = '\0'; 
//					printf("%s","String received from and resent to the client:");
					linenum++;   //linum may be maintained in a file
					strcpy(originInput,input);
//					char msg[] = "\nString received from and resent to the client:";
//					write(2,msg,strlen(msg)+1);
//					write(2,input,strlen(input)+1);
					fflush(stdout);   //?
					dup2(newsockfd,1);
					dup2(newsockfd,2);
					parseInput(newsockfd);
//					printf("n:%d\n",n);
//					if(flag == true)
//						dup2(newsockfd,1);
//					if (error == true){
//						send(newsockfd,"noooo\n",7,0);
//						error = false;
//					}
					
//					puts(str);
//					send(newsockfd,str,n,0);
//					write(2,str,strlen(str)+1);
//					fflush(stdout);
					send(newsockfd,"% ",2,0);
					memset(buf, 0, sizeof(buf));
					memset(input, 0, sizeof(input));
				} 
	
				exit(0);
				
		}
		else
		{          /*parent*/	
			close(newsockfd);
		}
	}
	
}


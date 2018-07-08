#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>

#pragma warning

#define MaxPipeNum 1000
#define MaxInputChar 15010
#define MaxCmdLength 256

void parseInput();
void parseCommand(char* cmd);
void handleCommand(char* cmd,char** subcmd);

char originInput[MaxInputChar];
char input[MaxInputChar];
char remain[MaxInputChar];

int newsockfd1;
int cmdnum = 0;
int linenum = 0;
int Inlinepipe1[2] ;
int Inlinepipe2[2] ;
//bool previousProcessExist = false;
int nowOffset = 0; 
bool isFirstProcess = false;
int debugmode = 0;
bool error = false;

struct Pipe{
	int pfds[2];
	bool exist;
};

Pipe pipearray[MaxPipeNum];


void parseInput(int sockfd)
{ 
	error = false;
	cmdnum = 0;
	nowOffset = 0;
//	write(2,"originInput:",12);
//	write(2,input,strlen(input));
//	write(2,"\n",1);

	newsockfd1 = sockfd;
	
	char *str1, *cmd, *saveptrProcess;
	char *sep;
	strcpy(remain,input);
	for(str1 = input;;str1 = NULL){
		sep = strpbrk(remain,"|!>");
		cmd = strtok_r(str1,"|!>\r\n",&saveptrProcess);
		if(cmd == NULL) break;
		else
			cmdnum++;
		
		if(cmdnum == 1)
			isFirstProcess = true;
		else 
			isFirstProcess = false;			
		parseCommand(cmd);
		if(error)
			return;
//		write(2,"back\n",5);
		if(sep != NULL){
//			int offset  = strspn(originInput+nowOffset+strlen(cmd),"|!> ");
			int offset;
			char* sepcmd = strpbrk(originInput+nowOffset+strlen(cmd),"|!>");
			offset = sepcmd -(originInput+nowOffset+strlen(cmd));
			offset += strspn(sepcmd,"|!> ");
			nowOffset += strlen(cmd)+offset;
			strcpy(remain,sep+1);
		}
	}

}

void parseCommand(char* cmd)
{
	char originCmd[MaxCmdLength];
	strcpy(originCmd,cmd);
	char *subcmd[200];
	char *str2, *chunk, *saveptrSubprocess;

	for(int j = 0;j < 200;j++)
		subcmd[j] = NULL;
	int k;
	for(str2 = cmd,k = 0;;str2 = NULL,k++){
		chunk = strtok_r(str2," ",&saveptrSubprocess);
		if(chunk == NULL)
			break;
		subcmd[k] = chunk; 
	}
	
	handleCommand(originCmd,subcmd);
}

void handleCommand(char* cmd,char** subcmd)
{
	//if( linenum>900){
	if(debugmode){	
		char title[120];  
		sprintf(title,"\nIn handleCommand,cmd:%s,subcmd1:%s,subcmd2:%s,linenum:%d,cmdnum:%d,nowOffset:%d\n",cmd,subcmd[1],subcmd[2],linenum,cmdnum,nowOffset);
		write(2 ,title,strlen(title));
		
		char cexist[25];
		sprintf(cexist,"pipearray[%d].exist:%d\n",(linenum-1)%MaxPipeNum,pipearray[(linenum-1)%MaxPipeNum].exist);
		write(2,cexist,strlen(cexist));
	
							char cfds1[100];
					sprintf(cfds1,"pipearray[%d].pfds:%d,%d\n",(linenum-1)%MaxPipeNum,pipearray[(linenum-1)%MaxPipeNum].pfds[0],pipearray[(linenum-1)%MaxPipeNum].pfds[1]);
					write(2,cfds1,strlen(cfds1));
		
				
		
					char ppipeNUM[30];
					sprintf(ppipeNUM,"p1[0]:%d p1[1]:%d p2[0]:%d p2[1]:%d\n",Inlinepipe1[0],Inlinepipe1[1],Inlinepipe2[0],Inlinepipe2[1]);
					write(2,ppipeNUM,strlen(ppipeNUM)); 
    }
//		printf("pipearray[999] pfds[1] is %d, pfds[0] is %d\n", 
//					pipearray[999].pfds[1],
//					pipearray[999].pfds[0]);
//		printf("%d\n", pipearray[999].exist);
	//}
	

	if(!strcmp(subcmd[0],"exit")){
		exit(0);
	}
	char leftchar;
	int i ;
	if(nowOffset == 0) leftchar = '\0';
	else{
		for(i = nowOffset-1;i >0 ;i--) 
			if(originInput[i] != ' ')break;
				leftchar = originInput[i];
	}
	if(leftchar == '>')return;
	/*
	char tmp[15];
	sprintf(tmp,"leftchar:%c\n",leftchar);
	write(2,tmp,strlen(tmp));
	*/
	if(!strcmp(subcmd[0],"printenv")){
		char *value;
		value = getenv(subcmd[1]);
		if(value != NULL){
			char envMsg[30];
			sprintf(envMsg,"%s=%s\n",subcmd[1],value);
			write(newsockfd1,envMsg,strlen(envMsg));
		}
		return;
	}
	else if(!strcmp(subcmd[0],"setenv")){
//		int set ;
		setenv(subcmd[1],subcmd[2],true); 
//		char a[1];
//		sprintf(a,"%d",set);
//		write(2,a,strlen(a)+1);
		return;
	}
	
	else if(isdigit(subcmd[0][0])){  //cmd is a number
//		write(2,subcmd[0],2);
//      here!!
		/*
		char *cmdpositionnum = strstr(origin,cmd);	
		char *left;
		if(cmdpositionnum != remain)
			left = cmdpositionnum-1;
		else 
		if(left[0] == '|' || left[0] == '!')   //left is  |
		{
		*/	
		if((nowOffset >0) && (originInput[nowOffset-1] == '|'||originInput[nowOffset-1] == '!'))
			return;
//		}
		
	}
	else {                      //exec()
//		printf("#1\n");		
		char* cmdposition = strstr(remain,cmd);
//		cmdposition = strstr(remain,cmd);
//		printf("remain:%s\n",remain);
//		printf("cmdposition:%s\n",cmdposition);
//		printf("#2\n");
		char* rightpos = cmdposition + strlen(cmd);
//		printf("\nrightpos#1:%s\n",rightpos);
//		printf("#3\n");
		while(rightpos != NULL  )
		{	
//			printf("in!!\n");
			if(strncmp(rightpos," ",1)){
				break;
			}
			rightpos++;
		}
//		printf("#4\n");
//		printf("\noriginInput:%s",originInput);
//		printf("\nrightpos:%c %c\n",rightpos[0],rightpos[1]);
//		printf("\nrightpos##1:%s\n",rightpos);
		int fd = -1,pipenum,errpipenum;
		char *outposition = NULL;
		char *errposition = NULL;
	
		if(rightpos!=NULL &&rightpos[0] == '>'){
//			write(2,"here\n",5);
			char filename[256];
			
			if(rightpos[1]==' ')
			{ 
				strcpy(filename,rightpos+strspn(rightpos+1," ")+1);
//				filename = rightpos+strspn(rightpos+1," ")+1;	 
			}
			else
			{
				strcpy(filename,rightpos+1);
			}
//			char test[2];
//			sprintf(test,"length:%d\n",strlen(filename));
//			write(2,test,strlen(test));
//			write(2,filename,strlen(filename));
//			filename[strlen(filename)-1] = '\0';
//			write(2,"#####",5);
//			sprintf(test,"length:%d\n",strlen(filename));
//			write(2,test,strlen(test));
//			write(2,filename,strlen(filename));
//			write(2,originInput,strlen(originInput));
//			return;
			char *ff = strtok(filename,"\r\n");
			
			fd = open(ff,O_WRONLY|O_TRUNC|O_CREAT,0666);
			if(fd == -1){
				fprintf(stderr, "Open File Failed: %s",strerror(errno));
				exit(-1);
			}
			dup2(fd,1);
		}
		

		
		else{
			
			outposition = strchr(rightpos,'|');
			errposition = strchr(rightpos,'!');
			
			
			
			if(rightpos != NULL && outposition != NULL){   //right has |
				if(debugmode == 1)
				write(2,"right#1\n",8);
				if(isdigit(outposition[1])){				//output to process in the next N line(|N)
					if(debugmode == 1)
					write(2,"right#2\n",8);
					if(errposition == NULL ) //|
					{
						pipenum = atoi(outposition+1);
						if(debugmode == 1)
						write(2,"right#3\n",8);
					}
					else if((errposition -outposition) < 0) //!N|N
						pipenum = atoi(outposition+1);
					else {                         //|N!N
						errposition[0] = '\0';
						pipenum = atoi(outposition+1);
						errposition[0] = '!';
					}
					if(pipearray[(linenum+pipenum-1)%MaxPipeNum].exist == false){
						if(debugmode == 1)
						write(2,"right#4\n",8);
						if(pipe(pipearray[(linenum+pipenum-1)%MaxPipeNum].pfds) < 0)
							exit(-1);
						else{
							pipearray[(linenum+pipenum-1)%MaxPipeNum].exist = true;
	//						printf("have pipe\n");
						}
					}
					if(debugmode == 1){
						char cfds[100];
						sprintf(cfds,"pipearray[%d].pfds:%d,%d\n",(linenum+pipenum-1)%MaxPipeNum,pipearray[(linenum+pipenum-1)%MaxPipeNum].pfds[0],pipearray[(linenum+pipenum-1)%MaxPipeNum].pfds[1]);
					
						write(2,cfds,strlen(cfds));
					}
				}
				else{     //output to next process in the same line(|)
//					write(2,"right#2\n",8);
					errposition = NULL;
					if(cmdnum%2){
						
						if(Inlinepipe1[0]!=-1){ 
						close(Inlinepipe1[0]);
						Inlinepipe1[0] = -1;
						}
						if(Inlinepipe2[1]!=-1){
						close(Inlinepipe2[1]);
						Inlinepipe2[1] = -1;
						}
					
						if(pipe(Inlinepipe1) < 0){
							perror("pipe");
							exit(EXIT_FAILURE); 
						}
					}
					else{
						
						if(Inlinepipe2[0]!=-1){ 
						close(Inlinepipe2[0]);
						Inlinepipe2[0] = -1;}
						if(Inlinepipe1[1]!=-1){
						close(Inlinepipe1[1]);
						Inlinepipe1[1] = -1;}
						if(pipe(Inlinepipe2) < 0){
							perror("pipe");
							exit(EXIT_FAILURE);
						}
	//					printf("right#5\n"); 
//						write(2,"right#3\n",8);
						if(debugmode == 1)
						write(2,"right#3\n",8);	
					}
					

					
				}
			}
		
			if(rightpos != NULL && errposition != NULL){   //right has !
	//			printf("right#3\n");
	//			printf("rightpos:%s\n",rightpos+1);
	//			sscanf(rightpos,"|%d",pipenum);
				if(outposition == NULL ) //!
					errpipenum = atoi(errposition+1);
				else if((errposition -outposition) > 0) //|N!N
					errpipenum = atoi(errposition+1);
				else {                         //!N|N
					outposition[0] = '\0';
					errpipenum = atoi(errposition+1);
				}
				if(pipearray[(linenum+errpipenum-1)%MaxPipeNum].exist == false){
					if(pipe(pipearray[(linenum+errpipenum-1)%MaxPipeNum].pfds) < 0)
						exit(-1);
					else{
						pipearray[(linenum+errpipenum-1)%MaxPipeNum].exist = true;
//						printf("have pipe, %d\n", pipearray[(linenum+errpipenum-1)%MaxPipeNum].exist);
					}
				}
			
			}
		
		}	
		
		
		
			
		
		/*
			write(2,"#5\n",4);
		

			
		
		char exist1[30],exist2[30],exist3[30];
		char fd1[30],fd2[30],fd3[30];
		
		sprintf(exist1,"pipearray[%d].exist:%d\n",linenum%MaxPipeNum-1,pipearray[linenum%MaxPipeNum-1].exist);
		sprintf(exist2,"pipearray[%d].exist:%d\n",linenum%MaxPipeNum-1+1,pipearray[linenum%MaxPipeNum-1+1].exist);
		sprintf(exist3,"pipearray[%d].exist:%d\n",linenum%MaxPipeNum-1+2,pipearray[linenum%MaxPipeNum-1+2].exist);
		
		sprintf(fd1,"pipearray[%d].pfds:%d,%d\n",linenum%MaxPipeNum-1,pipearray[linenum%MaxPipeNum-1].pfds[0],pipearray[linenum%MaxPipeNum-1].pfds[1]);
		sprintf(fd2,"pipearray[%d].pfds:%d,%d\n",linenum%MaxPipeNum-1+1,pipearray[linenum%MaxPipeNum-1+1].pfds[0],pipearray[linenum%MaxPipeNum-1+1].pfds[1]);
		sprintf(fd3,"pipearray[%d].pfds:%d,%d\n",linenum%MaxPipeNum-1+2,pipearray[linenum%MaxPipeNum-1+2].pfds[0],pipearray[linenum%MaxPipeNum-1+2].pfds[1]);
		
		write(2,exist1,strlen(exist1)+1);
		write(2,exist2,strlen(exist2)+1);
		write(2,exist3,strlen(exist3)+1);
		
		write(2,fd1,strlen(fd1)+1);
		write(2,fd2,strlen(fd2)+1);
		write(2,fd3,strlen(fd3)+1);
		
		*/
		
		pid_t pid;
		/*fork another process*/
		pid = fork();
		
		if(pid < 0){  /*error occurred */
			fprintf(stderr, "Fork Failed: %s",strerror(errno));
			exit(-1);
		}
		else if(pid == 0){    /*child process*/
			if(debugmode == 1)
			write(2, "In Child, beginning\n", strlen("In Child, beginning\n"));
//			printf("previousProcessExist:%d\n",previousProcessExist);
			if(!isFirstProcess){ 			//input from previous process in the same line(|) 				
//				printf("left#1\n");
				if(cmdnum%2){
					close(Inlinepipe2[1]);
					Inlinepipe2[1] = -1;
					dup2(Inlinepipe2[0],0);
//					close(Inlinepipe1[0]);
//					close(Inlinepipe2[0]);
//					printf("left#2\n");
				} 
				else{
					close(Inlinepipe1[1]);
					Inlinepipe1[1] = -1;
					dup2(Inlinepipe1[0],0);
//					close(Inlinepipe2[0]);
//					printf("left#3\n");	
				}
//				previousProcessExist = false;
//				write(2,"wrong\n",6);
			}
			else{
				if(pipearray[(linenum-1)%MaxPipeNum].exist){     //input from previous process in the previous line(|N)	
					close(pipearray[(linenum-1)%MaxPipeNum].pfds[1]);
					pipearray[(linenum-1)%MaxPipeNum].pfds[1] = -1;
					dup2(pipearray[(linenum-1)%MaxPipeNum].pfds[0], 0);
					//close(pipearray[(linenum-1)%MaxPipeNum].pfds[0]);
					if(debugmode == 1)
					write(2,"left#4\n",8);
				}
				else{
//					write(1,"left#5\n",8);
				}
			}
			
			if(rightpos!= NULL &&rightpos[0] == '>'){
//				write(2, "In Child, There is > \n", strlen("In Child, There is > \n"));
//				dup2(fd,1);
			}
			
			else{
				if(debugmode == 1)
				write(2, "In Child, No > \n", strlen("In Child, No > \n"));
				if(rightpos != NULL && outposition != NULL){ //right has |N 
					if(isdigit(outposition[1])){  //right |N
						dup2(pipearray[(linenum+pipenum-1)%MaxPipeNum].pfds[1],1);
						if(debugmode == 1)
						write(2,"right#5\n",9);
					}
					else{       //right |
	//					previousProcessExist = true;
	//					printf("right#7\n");
						if(cmdnum%2){
							dup2(Inlinepipe1[1],1);
//							close(Inlinepipe1[1]);
	//						printf("right#8\n");
						}
						else{
							dup2(Inlinepipe2[1],1);
//							close(Inlinepipe2[1]);
	//						printf("right#9\n");
						}
					}
					
				}
				if(rightpos != NULL && errposition != NULL){ 
					if(debugmode == 1)
					write(2, "In Child, dup2 pfds[1] to stderr", strlen("In Child, dup2 pfds[1] to stderr"));
					dup2(pipearray[(linenum+errpipenum-1)%MaxPipeNum].pfds[1],2); 
				}
				else{
//					write(2, "In Child, one of them is NULL", strlen("In Child, one of them is NULL"));
				}
			}
//	write(2,"#6\n",4);
		
		/*
		if(linenum == 3){
		char eexist1[30],eexist2[30],eexist3[30];
		char ffd1[30],ffd2[30],ffd3[30];	
		
		sprintf(eexist1,"pipearray[%d].exist:%d\n",linenum%MaxPipeNum-1,pipearray[linenum%MaxPipeNum-1].exist);
		sprintf(eexist2,"pipearray[%d].exist:%d\n",linenum%MaxPipeNum-1+1,pipearray[linenum%MaxPipeNum-1+1].exist);
		sprintf(eexist3,"pipearray[%d].exist:%d\n",linenum%MaxPipeNum-1+2,pipearray[linenum%MaxPipeNum-1+2].exist);
		
		sprintf(ffd1,"pipearray[%d].pfds:%d,%d\n",linenum%MaxPipeNum-1,pipearray[linenum%MaxPipeNum-1].pfds[0],pipearray[linenum%MaxPipeNum-1].pfds[1]);
		sprintf(ffd2,"pipearray[%d].pfds:%d,%d\n",linenum%MaxPipeNum-1+1,pipearray[linenum%MaxPipeNum-1+1].pfds[0],pipearray[linenum%MaxPipeNum-1+1].pfds[1]);
		sprintf(ffd3,"pipearray[%d].pfds:%d,%d\n",linenum%MaxPipeNum-1+2,pipearray[linenum%MaxPipeNum-1+2].pfds[0],pipearray[linenum%MaxPipeNum-1+2].pfds[1]);
		
		write(2,eexist1,strlen(eexist1)+1);
		write(2,eexist2,strlen(eexist2)+1);
		write(2,eexist3,strlen(eexist3)+1);
		
		write(2,ffd1,strlen(ffd1)+1);
		write(2,ffd2,strlen(ffd2)+1);
		write(2,ffd3,strlen(ffd3)+1);
		}
		*/		
			
			if(execvp(subcmd[0],subcmd) == -1){
//				cmdnum--;
				if(isFirstProcess)
					linenum--;
				//exit handleCommand??
			//	printf
				
				//exit(0);
				exit(3);
			}
				
			
		}
		else{ /*parent process*/ 
			if(fd != -1){
				close(fd);
			}
			else{
				if(debugmode == 1)	{
					char pipeNUM[30];
					sprintf(pipeNUM,"p1[0]:%d p1[1]:%d p2[0]:%d p2[1]:%d\n",Inlinepipe1[0],Inlinepipe1[1],Inlinepipe2[0],Inlinepipe2[1]);
					write(2,pipeNUM,strlen(pipeNUM)); 
				}
				if(rightpos != NULL && outposition!= NULL){
					if(isdigit(outposition[1])){
	//					close(pipearray[(linenum+pipenum-1)%MaxPipeNum].pfds[1]);
	//					printf("parent#1\n");
						if(debugmode == 1)
						write(2,"##parent!\n",strlen("##parent!\n"));		
					}
					else{
						if(cmdnum%2){
							if(Inlinepipe1[1]!=-1){
								close(Inlinepipe1[1]);
								Inlinepipe1[1] = -1;
							}
//							if(Inlinepipe2[0]!=-1)
//								close(Inlinepipe2[0]);
//							close(Inlinepipe1[0]);
//							close(Inlinepipe2[1]);
//							close(Inlinepipe2[0]);
	//						printf("parent#2\n");
						}
						
						else{
							if(Inlinepipe2[1]!=-1){
								close(Inlinepipe2[1]);
								Inlinepipe2[1] = -1;
							}
//							if(Inlinepipe1[0]!=-1)
//								close(Inlinepipe1[0]);
//							close(Inlinepipe2[0]);
//							close(Inlinepipe1[0]);
//							close(Inlinepipe1[1]);
	//						printf("parent#3\n");
						}
						 
					}
					 
				}
			}
			bool uoriginExist = pipearray[(linenum-1)%MaxPipeNum].exist;
			if(pipearray[(linenum-1)%MaxPipeNum].exist){     //input from previous process in the previous line(|N)	
				close(pipearray[(linenum-1)%MaxPipeNum].pfds[1]);
				pipearray[(linenum-1)%MaxPipeNum].pfds[1] = -1;
				//close(pipearray[(linenum-1)%MaxPipeNum].pfds[0]);
//				dup2(pipearray[linenum%MaxPipeNum-1].pfds[0],0);
				if(debugmode == 1)
				write(2,"parentleft#4\n",14);
				pipearray[(linenum-1)%MaxPipeNum].exist = false;
			}
//			write(2,"parent#1\n",10);
			//while(wait((int*)0)!=pid);  //error may occurr
			int status;
			while(wait(&status)!=pid);  //error may occur
			
			/*
			char cStatus[2];
			sprintf(cStatus, "%d %d %d %d %d\n", 
				WIFEXITED(status), 
				WIFSIGNALED(status), 
				WIFSTOPPED(status), 
				WIFCONTINUED(status), 
				WEXITSTATUS(status));
			write(1, cStatus,strlen(cStatus));
			*/
			if (WIFEXITED(status) && WEXITSTATUS(status)!=0 ) {
				char command[MaxCmdLength];
				char *ccmd = strtok(cmd," ");
				strcpy(command,"Unknown command: [");
				strcat(command,ccmd);
				strcat(command,"].\n");
				write(newsockfd1,command,strlen(command)); 
				error = true;
				if(isFirstProcess){
					pipearray[(linenum-1)%MaxPipeNum].exist = uoriginExist;
					uoriginExist = false;
					linenum--;
/*
					if(pipearray[linenum%MaxPipeNum].exist == false){
						pipe(pipearray[linenum%MaxPipeNum].pfds);
						pipearray[linenum%MaxPipeNum].exist = true;	
					}
					dup2(0,pipearray[linenum%MaxPipeNum].pfds[1]);
					close(pipearray[linenum%MaxPipeNum].pfds[1]);
*/					
				}
				else return;
				
			}
			else{
				
			}
			
			
		}
		
	
	}	
	
}
/*
int main()
{
	while(1){
		printf(">");
		fgets( input, MaxInputChar, stdin)  ;
		input[strlen(input)-1] = '\0';
		linenum++;
		strcpy(originInput,input);
		parseInput();
	}
}
*/
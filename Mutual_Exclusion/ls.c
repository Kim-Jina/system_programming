#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>

#define BUFFSIZE 1024
#define PORTNO 40084

typedef struct node{		// node struct
	int pid;		// child process ID
	int status;		// child process's status
	struct node* next;	// next node pointer
}NODE;

struct sockaddr_in c_addr;	// client address
int ppid;			// parent process pid
static pid_t pids;		// keep child's pid
static char *buf;		// Buffer
char h_name[1000];		// host name
struct hostent *e_host;		// hostent struct variable
NODE* pHead=NULL;		// Node's head pointer
int fd;				// file descriptor
int addr_len;			// address length
int maxNchildren;		// the maximum # of child process
int maxNspareserver;		// the maximum # of idle child process
int minNspareserver;		// the minimum # of idle child process
int startNserver;		// the # of excuting child process
int idle;			// the # of idle child process
int cpid;			// child process pid
char ip[1000]={};		// IP address
int port;			// port number
char path_ok[1000]={};		// path & whether forbiden or not
char log_s[1000]={};		// string variable
sem_t *mysem;			// semaphore

pid_t child_make(int socketfd,int addrlen);		// make child process
void child_main(int socketfd,int addrlen);		// connect child process
void sig_handler(int signo);				// signal handler
void print_t();						// print time function
void insert(int Pid, int Status);			// insertion function
void delete();						// delete function
void ls_print(int client_fd, char* host, char* temp, char* current_path);	// ls function

void *doit3(void *vptr){	// writing shared memory function
	FILE *file=fopen("access.log","a");		// file structure variable
	char string[1000]={};				// string variable

	strcpy(string,(char *)vptr);
	mysem=sem_open("40084",O_RDWR);			// semaphore open
	sem_wait(mysem);				// lock semaphore
	fprintf(file,"%s",string);			// print string
	sem_post(mysem);				// unlock semaphore
	fclose(file);					// close file
	sem_close(mysem);				// close semaphore
	memset(log_s,0,1000);				// initialize log_s string
	return NULL;
}
void count_id(){	// counting idle server function
	NODE* pNode=pHead;	// node variable
	int i_cnt=0;		// idle process counter
	// count idle server
	while(pNode!=NULL){
		if(pNode->status==0)	// if pNodes's status is 0(idle server)
			i_cnt++;	// increase idle process counter
		pNode=pNode->next;	// pNode is pNode's next node
	}
	// print information of idle server count
	print_t();
	printf("idleServerCount : %d\n",i_cnt);
	sprintf(log_s,"%sidleServerCout : %d\n",log_s,i_cnt);
	idle=i_cnt;				// save the # of idle process
}
void change_status(int Pid, int Status){	// changing status function
	NODE* pNode=pHead;			// node variable

	// find node to change status
	while(pNode!=NULL){
		if(pNode->pid==Pid)		// pNode's pid and Pid are same
			pNode->status=Status;	// change status
		pNode=pNode->next;		// pNode is pNode's next node
	}
}
void *doit1(void *vptr){	// writing shared memory function
	NODE *pNode=pHead;	// node variable
	pthread_t tid;		// thread
	int shm_id;		// shared memory id
	void *shm_addr;		// shared memory address
	// Each thread fetches, prints, and increments
	// the counter NLOOP times. The value of the counter
	// should increase monotonically.
	if((shm_id=shmget((key_t)PORTNO,BUFFSIZE,IPC_CREAT|0666))==-1){		// create shared memory
		printf("shmget fail\n");	// print fail message
		sprintf(log_s,"shmget fail\n");	// save string
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
		return NULL;
	}
	if((shm_addr=shmat(shm_id,(void *)0,0))==(void *)-1){			// attach shared memory to process
		printf("shmat fail\n");		// print fail message
		sprintf(log_s,"shmat fail\n");	// save string
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
		return NULL;
	}

	mysem=sem_open("40084",O_RDWR);			// semaphore open
	sem_wait(mysem);				// lock semaphore
	usleep(10000);					// sleep
	sprintf((char *)shm_addr,"%d/%s/%d",getpid(),inet_ntoa(c_addr.sin_addr),ntohs(c_addr.sin_port));	// share child process information
	sem_post(mysem);				// unlock semaphore
	sem_close(mysem);				// close semaphore
	usleep(10000);					// sleep

	return NULL;
}
void *doit2(void *vptr){
	pthread_t tid;		// thread
	int shm_id;		// shared memory id
	void *shm_addr;		// shared memory address
	char prev[32]={};	// string
	char *token;		// token

	if((shm_id=shmget((key_t)PORTNO,BUFFSIZE,IPC_CREAT|0666))==-1){		// create shared memory
		printf("shmget fail\n");		// print fail message
		sprintf(log_s,"shmget fail\n");	// save string
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
		return NULL;
	}
	if((shm_addr=shmat(shm_id,(void *)0,0))==(void *)-1){			// attach shared memory to process
		printf("shmat fail\n");			// print fail message
		sprintf(log_s,"shmat fail\n");	// save string
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
		return NULL;
	}
	
	strcpy(prev,(char *)shm_addr);	// copy shm_addr to prev
	token=strtok(prev,"/");		// token string
	cpid=atoi(token);		// store child process pid
	token=strtok(NULL,"/");		// token string
	strcpy(ip,token);		// store ip address
	token=strtok(NULL,"\n");		// token string
	port=atoi(token);		// store port number

	if(shmctl(shm_id,IPC_RMID,0)==-1){	// read shared memory or remove shared memory
		printf("shmctl fail\n");	// print fail message
		sprintf(log_s,"shmctl fail\n");	// save string
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
	}

	return NULL;
}
int main(int argc,char* argv[]){
	FILE *File;					// File variable
	pthread_t tid;					// thread
	struct sockaddr_in server_addr,client_addr;	// server address, client address
	int socket_fd,addrlen,i,opt=1;			// socket file descriptor, address's length, for for statement, for setsockopt function
	char f_str[1000]={};				// string (get file's information)
	char *token;					// token
	int c=0;					// count						// for for statement
	
	// signal handling
	signal(SIGINT,sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGCHLD,sig_handler);
	signal(SIGUSR1,sig_handler);

	mysem=sem_open("40084",O_CREAT|O_RDWR,0700,1);	// open semaphore
	sem_close(mysem);				// close semaphore

	print_t();				// print time function
	printf("Server is started.\n");		// print message
	sprintf(log_s,"%sServer is started.\n",log_s);	// save string

	buf=(char *)malloc(BUFFSIZE+1);		// buf is dynamically allocated by BUFFSIZE+1

	if((socket_fd=socket(PF_INET,SOCK_STREAM,0))<0){	// create a socket
		printf("Server: Can't open stream socket.\n");
		sprintf(log_s,"%sServer: Can't open stream socket.\n",log_s);	// save string
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
		return 0;
	}

	setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));	// use setsockopt function to block bind error
	bzero((char *)&server_addr,sizeof(server_addr));
	server_addr.sin_family=AF_INET;			// receive address family
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);	// receive IPv4 address
	server_addr.sin_port=htons(PORTNO);		// receive port number

	if(gethostname(h_name,sizeof(h_name))==0)		// get host name
		 e_host=gethostbyname(h_name);			// get host's information

	print_t();				// print time function
	printf("Socket is created. IP: %s, port: %d\n",inet_ntoa(*(struct in_addr*)e_host->h_addr_list[0]),(int)ntohs(server_addr.sin_port));		// print message
	sprintf(log_s,"%sSocket is created. IP: %s, port: %d\n",log_s,inet_ntoa(*(struct in_addr*)e_host->h_addr_list[0]),(int)ntohs(server_addr.sin_port));	// save string
	
	if(bind(socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){	// associate an address with a socket 
		printf("Server: Can't bind local address.\n");
		sprintf(log_s,"%sServer: Can't bind local address.\n",log_s);	// save string
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
		return 0;
	}
	listen(socket_fd,5);		// announce that server is willing to accept connect request

	// --------------------- read file --------------------------------------------
	File=fopen("httpd.conf","r");	// file open
	if(File==0){	// if file doesn't exist
		printf("no file\n");
		sprintf(log_s,"%sno file\n",log_s);
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
		return;
	}
	else{	// if file exists
		while(fgets(f_str,1000,File)){	// read file
			for(i=0;f_str[i]!='\0';i++){	// change new line character to null
				if(f_str[i]=='\n'){
					f_str[i]='\0';
					break;
				}
			}
			// token file's informaion
			token=strtok(f_str,": ");
			token=strtok(NULL," \t");
			if(c==0)					// if file's information is maxchildren
				maxNchildren=atoi(token);		// receive maxNchildren
			else if(c==1)					// if file's information is maxspareserver
				maxNspareserver=atoi(token);		// receive maxNspareserver
			else if(c==2)					// if file's information is minspareserver
				minNspareserver=atoi(token);		// receive minNspareserver
			else						// if file's information is startserver
				startNserver=atoi(token);		// receive startNserver
			c++;						// increase variable
		}
		fclose(File);	// close file
	}
	
	ppid=getpid();			// receive parent process pid
	addrlen=sizeof(client_addr);	// save client_address's size into addrlen

	fd=socket_fd;			// save socket file descriptor
	addr_len=addrlen;		// save client's address length
	
	pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
	pthread_join(tid,NULL);					// wait thread

	// Pre-forking routine
	for(i=0;i<startNserver;i++){
		pids=child_make(socket_fd,addrlen);	// parent returns
	}
	
	pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
	pthread_join(tid,NULL);					// wait thread
	
	for(;;)
		pause();	// pause program

	return 0;	// end of program
}
pid_t child_make(int socketfd,int addrlen){		// make child process
	pid_t pid;
	pthread_t tid;				// thread
	if((pid=fork())>0){	// parent process
		insert(pid,0);	// insertion function(make linked list)
		print_t();	// print time function
		printf("%d process is forked.\n",(int)pid);	// print message
		sprintf(log_s,"%s%d process is forked.\n",log_s,(int)pid);	// save string
		count_id();					// go to count_id function
		return(pid);	// parent move out
	}

	child_main(socketfd,addrlen);	// go to child process to excute child process
}
void child_main(int socketfd,int addrlen){		// connect child process
	pthread_t tidA,tid;				// thread
	FILE *File;					// FILE struct variable
	int client_fd,len_out;				// client file descriptor, length
	char buf[BUFFSIZE];				// buffer
	socklen_t clilen;				// client address's length
	struct sockaddr_in client_addr;			// client address
	time_t n_time;					// current time
	int v_len,h_len,t_len;				// length
	char version[1000]={};				// version
	char host[1000]={};				// host
	char temp[1000]={};				// temp
	char *t_host;					// token host
	char w_str[1000]={};				// write string
	char current_path[1000]={};			// current path
	char f_str[1024]={};				// file string
	int i,j,k1,k2;					// variables for for
	int check;					// check wild card	
	
	signal(SIGINT, SIG_IGN);	// ignore SIGINT(ctrl+c)
	signal(SIGTERM, SIG_DFL);	// default SIGTERM

	while(1){
		clilen=addrlen;		// save client_address's size into clilen
		client_fd=accept(socketfd,(struct sockaddr*)&client_addr,&clilen);	// accept a connect request from client
		if(client_fd<0){	// if it cannot accept
			printf("Server: accept failed.\n");
			sprintf(log_s,"Server: accept failed.\n");	// save string
			pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
			pthread_join(tid,NULL);					// wait thread
			return;
		}

		// -------------------fnmatch----------------------------------
		File=fopen("accessible.usr","r");	// file open
		if(File==0){	// if file doesn't exist
			printf("no file\n");
			return;
		}
		else{	// if file exists
			check=1;	// check is 1
			while(fgets(f_str,1000,File)){	// read file
				for(i=0;f_str[i]!='\0';i++){	// change new line character to null
					if(f_str[i]=='\n'){
						f_str[i]='\0';
						break;
					}
				}
				if(fnmatch(f_str,inet_ntoa(client_addr.sin_addr),0)==0){	// match
					check=0;	// check is 0
					break;
				}
			}
			fclose(File);	// close file
		}

		if(check==1){	// not match, print error message
			strcpy(w_str,"HTTP/1.1 200 OK\r\nContent-Length: 1000000\r\nContent-Type: text/html\r\n\r\n");
			write(client_fd,w_str,strlen(w_str));
			sprintf(w_str,"<h3>Access denied!<br>Your IP : %s<br></h3>You have no permission to access this web server.<br>HTTP 403.6-Forbidden: IP address reject<br>",inet_ntoa(client_addr.sin_addr));
			write(client_fd,w_str,strlen(w_str));
			close(client_fd);	// close client descriptor
			exit(0);		// exit function
		}

		if((len_out=read(client_fd,buf,BUFFSIZE))>0){	// read HTTP request message
			if(strncmp(buf,"GET /favicon.ico",16)==0){	// if favicon.ico message operates
				close(client_fd);	// close client file descriptor
				continue;
			}
			// initialize host, version, temp
			memset(host,0,1000);
			memset(version,0,1000);
			memset(temp,0,1000);
			// find GET / HTTP/1.1
			for(i=0,v_len=0;i<len_out;i++){		// find starting point of 'G' 
				if(buf[i]=='G'){
					for(j=i;buf[j+1]!='\n';j++)	// save version
						version[v_len++]=buf[j];
					break;
				}
			}
			// find Host
			for(k1=j,h_len=0;k1<len_out;k1++){		// find starting point of 'H' 
				if(buf[k1]=='H'){
					for(k2=k1;buf[k2+1]!='\n';k2++)	// save host
						host[h_len++]=buf[k2];
					// strtok host
					t_host=strtok(host,":");
					t_host=strtok(NULL," ");
					strcpy(host,t_host);
					break;
				}
			}
			// find temp
			for(i=0,t_len=0;i<v_len;i++){
				if(version[i]=='/'){	// find starting point of '/'
					if(version[i+1]!=' '){	// exist temp
						for(j=i;version[j]!=' ';j++)	// save temp
							temp[t_len++]=version[j];
					}
					break;
				}
			}			
			if(temp[t_len-1]=='/')	// if temp's last letter is '/'
				temp[t_len-1]='\0';	// temp's last letter is NULL
			// ---------------------------------------ls---------------------------------------------
			getcwd(current_path,10000); // get current working directory path
			if(t_len!=0)	// if exist relative path
				strcat(current_path,temp);	// add relative path to current working directory path
			ls_print(client_fd,host,temp,current_path);	// go to ls_print function
			c_addr=client_addr;			// save client address

			// print and save new client's information
			printf("============ New client ============\n");
			sprintf(log_s,"============ New client ============\n");
			print_t();
			printf("%s\nIP : %s\nPort : %d\n",path_ok,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
			printf("=====================================\n");
			sprintf(log_s,"%s%s\nIP : %s\nPort : %d\n=====================================\n",log_s,path_ok,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
			pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
			pthread_join(tid,NULL);					// wait thread

			pthread_create(&tidA,NULL,&doit1,NULL);	// create thread and go to doit1 function to make shared memory
			pthread_join(tidA,NULL);		// wait for thread to terminate
			kill(ppid,SIGUSR1);			// use kill to make signal
		}
		close(client_fd);	// close client file descriptor
		usleep(10000);
		// print and save disconnected client's information
		printf("======== Disconnected client ========\n");
		sprintf(log_s,"======== Disconnected client ========\n");
		print_t();	// print time
		printf("%s\nIP : %s\nPort : %d\n",path_ok,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		printf("=====================================\n");
		sprintf(log_s,"%s%s\nIP : %s\nPort : %d\n=====================================\n",log_s,path_ok,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread

		pthread_create(&tidA,NULL,&doit1,NULL);	// create thread and go to doit1 function to make shared memory
		pthread_join(tidA,NULL);			// wait for thread to terminate
		kill(ppid,SIGUSR1);			// use kill to make signal
	}
	close(socketfd);		// close socket file descriptor
}
void sig_handler(int signo){				// signal handler
	pid_t PID=0;				// process ID
	pthread_t tidA,tid;			// thread
	NODE *pNode, *pCur, *pNew, *pDel;	// node variable
	int i=4,j=3,s=0,e=0;			// integer variable(for for statement and for array)
	int shm_id=shmget((key_t)PORTNO,BUFFSIZE,IPC_CREAT|0666);	// shared memory id

	if(signo==SIGUSR1){		// if SIGUSR1 is operated
		pthread_create(&tidA,NULL,&doit2,NULL);	// create thread and go to doit1 function to make shared memory
		pthread_join(tidA,NULL);			// wait for thread to terminate
		
		pNode=pHead;				// pNode is pHead
		while(pNode!=NULL){
			if(pNode->pid==cpid){	// find child process pid
				if(pNode->status==1){ // find disconnected client
					change_status(cpid,0);			// go to change_status function
				}
				else{	// find connected client
					change_status(cpid,1);			// go to change_status function
				}
				count_id();				// go to count_id function
				pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
				pthread_join(tid,NULL);					// wait thread
			}
			pNode=pNode->next;		// pNode is pNode's next node
		}
		if(idle<maxNchildren){				// if the # of idle child process is less than the maximum # of child process
			if(idle<minNspareserver){		// if the # of idle child process is less than the minimum # of idle child process
				e=startNserver-idle;		// save result of startNserver-idle		
				for(s=0;s<e;s++)
					pids=child_make(fd,addr_len);	// go child_make function
			}
			if(idle>maxNspareserver){		// if the # of idle child process is more than the maximum # of idle child process
				e=idle-startNserver;		// save result of startNserver-idle
				for(s=0;s<e;s++){
					pDel=pHead;			// pDel is pHead
					while(pDel!=NULL){
						if(pDel->status==0)	// find idle server
							break;
						pDel=pDel->next;	// pDel is pDel's next node
					}
					kill(pDel->pid,SIGTERM);	// kill process
					usleep(10000);			// sleep
				}
			}
		}
	}
	else if(signo==SIGCHLD){	// if child process exits
		PID=waitpid(-1,NULL,WNOHANG);	// PID has child status by using wait function
		pNode=pCur=pHead;		// pNode and pCur are pHead
		while(pNode!=NULL){
			if(pNode->pid==PID){	// if pNode's pid and PID are same
				if(pNode==pHead)	// if pNode is pHead
					pHead=pNode->next;	// pHead is pNode's next node
				else			// if pNode isn't pHead
					pCur->next=pNode->next;	// pCur's next node is pNode's next node
				print_t();			// print time
				printf("%d process is terminated.\n",pNode->pid);	// print message
				sprintf(log_s,"%s%d process is terminated.\n",log_s,pNode->pid); // save string
				count_id();				// go to count_id function
				free(pNode);	// delete pNode
				break;	// end of while statement
			}
			pCur=pNode;		// pCur is pNode
			pNode=pNode->next;	// pNode is pNode's next node
		}
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
	}
	else if(signo==SIGINT){			// if ctrl+c is used	
		pNew=pDel=pHead;		// pNew and pDel are pHead	
		while(pNew!=NULL){		// make idle
			pNew->status=0;		// change status by 0
			pNew=pNew->next;	// pNew is pNew's next node
		}
		while(pDel!=NULL){
			kill(pDel->pid,SIGTERM);	// kill process
			print_t();			// print time
			printf("%d process is terminated.\n",pDel->pid);	// print message
			sprintf(log_s,"%s%d process is terminated.\n",log_s,pDel->pid);	// save string
			pNew=pDel;		// pNew is pDel
			pDel=pDel->next;	// pDel is pDel's next node
			pHead=pDel;		// pHead is pDel
			count_id();		// go to count_id function
		}
		if(shmctl(shm_id,IPC_RMID,0)!=-1){	// check whether shared memory is terminated
			print_t();
			printf("Shared memory is terminated.\n");
			sprintf(log_s,"%sShared memory is terminated.\n",log_s);
		}
		print_t();		// print time
 		printf("Server is terminated.\n");	// print message
		sprintf(log_s,"%sServer is terminated.\n",log_s);	// save string
		delete();	// delete linked list
		pthread_create(&tid,NULL,doit3,(void *)&log_s);		// create thread
		pthread_join(tid,NULL);					// wait thread
		sem_unlink("40084");					// unlink semaphore
 		exit(0);	// exit
	}
	else					// if process are terminated		
		exit(0);	// exit
}
void print_t(){						// print time function
	struct tm* s_time;	// time struct variable
	time_t c_time;		// time varible
	char* week[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};					// week
 	char* month[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};	// month
	
	// save current time
	time(&c_time);
	s_time=localtime(&c_time);

	// print time information
	printf("[%s %s %d %d:%d:%d %d] ",week[s_time->tm_wday],month[s_time->tm_mon],s_time->tm_mday,s_time->tm_hour,s_time->tm_min,s_time->tm_sec, s_time->tm_year+1900);
	sprintf(log_s,"%s[%s %s %d %d:%d:%d %d] ",log_s,week[s_time->tm_wday],month[s_time->tm_mon],s_time->tm_mday,s_time->tm_hour,s_time->tm_min,s_time->tm_sec, s_time->tm_year+1900);	// save string
}
void insert(int Pid, int Status){					// insertion function
	NODE* pNode=(NODE *)malloc(sizeof(NODE));	// make node

	// input node's information
	pNode->pid=Pid;		// store child process id
	pNode->status=Status;	// store child process status

	// insertion node(make linked list)
	if(pHead==NULL)			// if pHead is NULL
		pNode->next=NULL;	// node's next pointer is NULL
	else				// if pHead isn't NULL
		pNode->next=pHead;	// pNode's next is pHead
	pHead=pNode;			// pHead is pNode		
}
void delete(){						// delete function
	NODE* pNode;			// node pointer
	
	// delete linked list
	while(pHead!=NULL){
		pNode=pHead;		// pNode is pHead
		pHead=pNode->next;	// pHead is pNode's next node
		free(pNode);		// free pNode
	}
}
void ls_print(int client_fd, char* host, char* temp, char* current_path){	// ls function
	DIR *dirp;
	FILE *File;
	struct dirent *dir;
	struct stat  sta;
	char f_str[1024]={};				// file string
	char w_str[1000]={};				// write string
	char error[1000][256];				// error message
	int block[1000]={};				// block size
	char permission[1000][256];			// permission
	int linkcounter[1000]={};			// linkcounter
	char u_ID[1000][256];				// userID
	char g_ID[1000][256];				// group ID
	int capacity[1000]={};				// capacity
	int month[1000]={};				// month
	int day[1000]={};				// day
	int hour[1000]={};				// hour
	int minute[1000]={};				// minute
	char file[1000][256];				// file name
	char path[10000]={};				// directory path
	char info[10000]={};				// information
	char t_path[1000];				// token path
	char t_fn[1000];				// token wild card
	char tmp[256]={};				// swap string
	char w1[100]={};				// first word
	char w2[100]={};				// second word
	char c1,c2;					// first character, second character
	int total=0,h_total=0;				// overall total and hidden file total
	int i_index,w_index,d_index,index1,index2;	// index
	int l,l1,l2,e_len=0;				// words' length, error's length
	int i,j,s,k1,k2,s1,s2;				// integer variable(for for loop)
	int swap_i=0;					// swap interger
	int read_d,ok;					// read directory, check
	// store informationf
	int s_block[1000]={};				// block size
	char s_permission[1000][256];			// permission
	int s_linkcounter[1000]={};			// linkcounterf
	char s_u_ID[1000][256];				// userID
	char s_g_ID[1000][256];				// group ID
	int s_capacity[1000]={};			// capacity
	int s_month[1000]={};				// month
	int s_day[1000]={};				// day
	int s_hour[1000]={};				// hour
	int s_minute[1000]={};				// minute
	char s_file[1000][256];				// file name
	char s_path[1000][256];				// directory path
	int s_total[1000]={};				// total
	// save order and index	
	int num_path[1000]={};				// path order
	int num_total[1000]={};				// total order
	int num_start[1000]={};				// start order
	int num_end[1000]={};				// end order	
	int s_index=0,p_index=0,t_index=0,b_index=0,e_index=0;	// information index
	opterr=0;
	read_d=l=total=h_total=index1=index2=i_index=w_index=d_index=0; // initialize variables
	for(i=0;i<1000;i++){	// initialize error, permission,user ID, group ID, file, path
		memset(error[i],0,256);
		memset(s_permission[i],0,256);
		memset(s_u_ID[i],0,256);
		memset(s_g_ID[i],0,256);
		memset(s_file[i],0,256);
		memset(s_path[i],0,256);
		memset(permission[i],0,256);
		memset(u_ID[i],0,256);
		memset(g_ID[i],0,256);
		memset(file[i],0,256);
	}
	// ---------------------------------------start ls----------------------------------------------------
	dirp=opendir(current_path); // open directory
	if(dirp!=NULL){	// can read directory
		read_d=1; // can read directory
		while((dir=readdir(dirp))!=NULL){	
			index2=0;	// initialize variables
			strcpy(path,current_path);	// path = current path
			strcpy(info,path);	// receive info
			strcat(info,"/");
			strcat(info,dir->d_name);
			stat(info,&sta);
			strcpy(file[index1],dir->d_name); // file name
			// permission
			switch((sta.st_mode)&(S_IFMT)){ // information of file type
				case S_IFREG:	// file
					permission[index1][index2++]='-';
					break;
				case S_IFDIR:	// directory
					permission[index1][index2++]='d';
					break;
				case '?':
					break;
			}	
			for(i=0;i<3;i++){	// store user,group,others to permission
				if(sta.st_mode&(S_IREAD>>i*3))	// read permission information
					permission[index1][index2++]='r';
				else
					permission[index1][index2++]='-';
				if(sta.st_mode&(S_IWRITE>>i*3))	// write permission information
					permission[index1][index2++]='w';
				else
					permission[index1][index2++]='-';
				if(sta.st_mode&(S_IEXEC>>i*3))	// execute permission information
					permission[index1][index2++]='x';
				else
					permission[index1][index2++]='-';
			}
			permission[index1][index2]='\0';
			linkcounter[index1]=sta.st_nlink; // linkcounter
			strcpy(u_ID[index1],getpwuid(sta.st_uid)->pw_name); // user ID
			strcpy(g_ID[index1],getgrgid(sta.st_gid)->gr_name); // group ID
			capacity[index1]=sta.st_size; // capacity
			month[index1]=localtime(&sta.st_mtime)->tm_mon+1; // month
			day[index1]=localtime(&sta.st_mtime)->tm_mday;	// day
			hour[index1]=localtime(&sta.st_mtime)->tm_hour;	// hour
			minute[index1]=localtime(&sta.st_mtime)->tm_min; // minute
			block[index1++]=(int)sta.st_blocks/2;	// the number of 1K blocks
			total+=(int)sta.st_blocks/2; // overall total
		}
	}	
	if(read_d==1){ // if directory exist
		//---------------------Bubble sort(file name)--------------------------
		for(k1=0;k1<index1;k1++){
			for(k2=0;k2<index1-1;k2++){
				l1=l2=s1=s2=0;	// initialize variables
				strcpy(w1,file[k2]);	// first word
				strcpy(w2,file[k2+1]);	// second word
				if(strcmp(w1,w2)==0)	// if first word = second word
					continue;
				c1=w1[0];	// first character
				c2=w2[0];	// second character
				// calculate words' length
				while(w1[l1]!='\0')
					l1++;	
				while(w2[l2]!='\0')
					l2++;
				// ---------------hidden file or folder------------------
				if(c1=='.'||c1=='/'){	// first word
					if(l1==2){	// check whether parent folder or not
						if(w1[1]!='.'){
							c1=w1[1];
							s1=1;
						}
					}
					else{	// change character to compare
						c1=w1[1];
						s1=1;
					}	
				}
				if(c2=='.'||c2=='/'){	// second word
					if(l2==2){	// check whether parent folder or not
						if(w2[1]!='.'){
							c2=w2[1];
							s1=1;
						}
					}
					else{	// change character to compare
						c2=w2[1];
						s2=1;
					}
				}		
				s=1;	// initialize variable
				while(1){
					// change capital letter to small letter
					if(c1>='A'&&c1<='Z')
						c1+=32;
					if(c2>='A'&&c2<='Z')
						c2+=32;
					if(c1!=c2)
						break;
					else{	// first character = second character
						// compare l1 and l2
						if(l1>l2)
							l=l1;
						else
							l=l2;
						for(i=s;i<l;i++){	// find different letter
							if(s1==0&&s2==0){	// no hidden file
								c1=w1[i];
								c2=w2[i];
							}
							else if(s1==1&&s2==1){	// both hidden file
								c1=w1[i+1];
								c2=w2[i+1];
							}
							else if(s1==1&&s2==0){	// first word is hidden file
								c1=w1[i+1];
								c2=w2[i];
							}
							else{	// second word is hidden file
								c1=w1[i];
								c2=w2[i+1];
							}
							if(c1!=c2)
								break;
						}
						s1=s2=0;	// initialize variable(check whether hidden file or not)
					}
					s++;	// increase variable
				}
				if(c1>c2){	// first word's character > second word's character
					// change file's position
					strcpy(tmp,file[k2]);
					strcpy(file[k2],file[k2+1]);
					strcpy(file[k2+1],tmp);
					// change permission's position
					strcpy(tmp,permission[k2]);
					strcpy(permission[k2],permission[k2+1]);
					strcpy(permission[k2+1],tmp);
					// change linkcounter's position
					swap_i=linkcounter[k2];
					linkcounter[k2]=linkcounter[k2+1];
					linkcounter[k2+1]=swap_i;
					// change user ID's position
					strcpy(tmp,u_ID[k2]);
					strcpy(u_ID[k2],u_ID[k2+1]);
					strcpy(u_ID[k2+1],tmp);
					// change group ID's position
					strcpy(tmp,g_ID[k2]);
					strcpy(g_ID[k2],g_ID[k2+1]);
					strcpy(g_ID[k2+1],tmp);
					// change capacity's position
					swap_i=capacity[k2];
					capacity[k2]=capacity[k2+1];
					capacity[k2+1]=swap_i;
					// change month's position
					swap_i=month[k2];
					month[k2]=month[k2+1];
					month[k2+1]=swap_i;
					// change day's position
					swap_i=day[k2];
					day[k2]=day[k2+1];
					day[k2+1]=swap_i;
					// change hour's position
					swap_i=hour[k2];
					hour[k2]=hour[k2+1];
					hour[k2+1]=swap_i;
					// change minute's position
					swap_i=minute[k2];
					minute[k2]=minute[k2+1];
					minute[k2+1]=swap_i;
					// change block's position
					swap_i=block[k2];
					block[k2]=block[k2+1];
					block[k2+1]=swap_i;
				}
				// initialize words
				for(i=0;i<l1;i++)
					w1[i]='\0';
				for(i=0;i<l2;i++)
					w2[i]='\0';
			}		
		}			
		// save directory path and total
		num_path[p_index]=s_index;
		strcpy(s_path[p_index++],path);
		num_total[t_index]=s_index;
		s_total[t_index++]=total;		
		if(index1!=0)	// begin information
			num_start[b_index++]=s_index;
		// save information
		for(i=0;i<index1;i++,s_index++){
			strcpy(s_file[s_index],file[i]);		// save file 
			strcpy(s_permission[s_index],permission[i]);	// save permission
			strcpy(s_u_ID[s_index],u_ID[i]);		// save user ID
			strcpy(s_g_ID[s_index],g_ID[i]);		// save group ID
			s_block[s_index]=block[i];			// save block size
			s_linkcounter[s_index]=linkcounter[i];		// save linkcounter
			s_capacity[s_index]=capacity[i];		// save capacity
			s_month[s_index]=month[i];			// save month
			s_day[s_index]=day[i];				// save day
			s_hour[s_index]=hour[i];			// save hour
			s_minute[s_index]=minute[i];			// save minute
		}
		if(index1!=0)	// end information
			num_end[e_index++]=s_index;
	}	
	else	// no directory path
		strcpy(error[e_len++],current_path);
	closedir(dirp);	// close directory
	// --------------------------------print result---------------------------------------------------------------
	sprintf(path_ok,"%s ",current_path);	// save current path
	// print error file or directory
	for(i=0;i<e_len;i++){
		File=fopen(current_path,"r");	// file open
		if(File==0){	// if file doesn't exist
			// send HTTP response meessage to client
			strcpy(w_str,"HTTP/1.1 200 OK\r\nContent-Length: 10000\r\nContent-Type: text/html\r\n\r\n");
			write(client_fd,w_str,strlen(w_str));
			// print title and head
			sprintf(w_str,"<html><head><title>%s</title></head>",current_path);
			write(client_fd,w_str,strlen(w_str));
			sprintf(w_str,"ls: cannot access %s: No such file or directory<br>",error[i]);
			write(client_fd,w_str,strlen(w_str));
			sprintf(path_ok,"%s403 Forbidden",path_ok);	// save string
		}
		else{	// if file exists
			strcpy(w_str,"HTTP/1.1 200 OK\r\nContent-Length: 1000000\r\nContent-Type: text/plain\r\n\r\n");
			write(client_fd,w_str,strlen(w_str));
			while(fgets(f_str,1000,File)){	// read file
				write(client_fd,f_str,strlen(f_str));
			}
			fclose(File);
			sprintf(path_ok,"%s200 OK",path_ok);	// save string
		}
	}
	if(e_len!=0)
		return;
	sprintf(path_ok,"%s200 OK",path_ok);	// save string

	// send HTTP response meessage to client
	strcpy(w_str,"HTTP/1.1 200 OK\r\nContent-Length: 10000\r\nContent-Type: text/html\r\n\r\n");
	write(client_fd,w_str,strlen(w_str));
	// print title and head
	sprintf(w_str,"<html><head><title>%s</title></head>",current_path);
	write(client_fd,w_str,strlen(w_str));

	j=s=k1=k2=0;	// initialize variable		
	// print information
	for(i=0;i<s_index;i++){
		ok=0;	// initialize variable
		if(num_path[k1]==i&&p_index!=0){	// print directory path
			sprintf(w_str,"<h3>Directory path: %s</h3>",s_path[k1++]);
			write(client_fd,w_str,strlen(w_str));
		}
		if(num_total[k2]==i&&t_index!=0){	// print total
			sprintf(w_str,"<h3>total %d</h3>",s_total[k2]);			
			write(client_fd,w_str,strlen(w_str));
			if(s_total[k2]==0)	// total is 0
				ok=1;
			k2++;	// increase variable
		}
		if(num_start[j]==i&&ok==0){	// make table
			sprintf(w_str,"<table border='1'><tr><th>File Name</th><th>Permission</th><th>Link</th><th>Owner</th><th>Group</th><th>Size</th><th>Last Modified</th></tr><tr>");
			write(client_fd,w_str,strlen(w_str));
			j++;	// increase variable
		}
		// print file information
		sprintf(w_str,"<td><a href=http://%s%s/%s>%s</a></td>",host,temp,s_file[i],s_file[i]);
		write(client_fd,w_str,strlen(w_str));
		sprintf(w_str,"<td>%s</td><td>%d</td><td>%s</td><td>%s</td><td>%d</td><td>%d %d %d:%d</td></tr>",s_permission[i],s_linkcounter[i],s_u_ID[i],s_g_ID[i],s_capacity[i],s_month[i],s_day[i],s_hour[i],s_minute[i]);
		write(client_fd,w_str,strlen(w_str));
		if(num_end[s]==i+1){	// close table
			sprintf(w_str,"</table>");
			write(client_fd,w_str,strlen(w_str));
			s++; // increase variable
		}
	}
}

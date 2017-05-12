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

#define BUFFSIZE	1024
#define PORTNO		40084

void ls_print(int client_fd, char* host, char* temp, char* current_path){
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
	// print error file or directory
	for(i=0;i<e_len;i++){
		File=fopen(current_path,"r");	// file open
		if(File==0){	// if file doesn't exist
			// send HTTP response meessage to client
			strcpy(w_str,"HTTP/1.1 404 NOT FOUND\r\nContent-Length: 10000\r\nContent-Type: text/html\r\n\r\n");
			write(client_fd,w_str,strlen(w_str));
			// print title and head
			sprintf(w_str,"<html><head><title>%s</title></head>",current_path);
			write(client_fd,w_str,strlen(w_str));
			sprintf(w_str,"ls: cannot access %s: No such file or directory<br>",error[i]);
			write(client_fd,w_str,strlen(w_str));
		}
		else{	// if file exists
			strcpy(w_str,"HTTP/1.1 200 OK\r\nContent-Length: 1000000\r\nContent-Type: text/plain\r\n\r\n");
			write(client_fd,w_str,strlen(w_str));
			while(fgets(f_str,1000,File)){	// read file
				write(client_fd,f_str,strlen(f_str));
			}
			fclose(File);
		}
	}
	if(e_len!=0)
		return;
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
int main(int argc, char **argv){   
	struct sockaddr_in server_addr,client_addr;
	int socket_fd,client_fd;			// socket file descriptor, client file descriptor
	int len,len_out,v_len,h_len,t_len;		// length
	char buf[BUFFSIZE];				// buffer
	char s_buf[BUFFSIZE];				// send buffer
	char version[1000]={};				// version
	char host[1000]={};				// host
	char temp[1000]={};				// temp
	char *t_host;					// token host
	char w_str[1000]={};				// write string
	char current_path[1000]={};			// current path
	int opt=1;					// socket option
	int i,j,k1,k2;					// variables for for
	// -----------------------------------server-------------------------------------------------
	if((socket_fd=socket(PF_INET,SOCK_STREAM,0))<0){	// create a socket	
		printf("Server: Can't open stream socket.");
		return 0;
	}
	bzero((char *)&server_addr,sizeof(server_addr));
	server_addr.sin_family=AF_INET;	// receive address family
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);	// receive IPv4 address
	server_addr.sin_port=htons(PORTNO);	// receive port number
	setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));	// use setsockopt function to block bind error
	if(bind(socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){	// associate an address with a socket 
		printf("Server: Can't bind local address.\n");
		return 0;
	}
	listen(socket_fd,5);	// announce that server is willing to accept connect request
	while(1){
		len=sizeof(client_addr);	// save client_address's size into len
		client_fd=accept(socket_fd,(struct sockaddr*)&client_addr,&len);	// accept a connect request from client
		if(client_fd<0){	// if it isn't accept
			printf("Server: accept failed.\n");
			return 0;
		}
		printf("[%d:%d] client was connected.\n",client_addr.sin_addr.s_addr,client_addr.sin_port);
		if((len_out=read(client_fd,buf,BUFFSIZE))>0){	// read HTTP request message
			// initialize host, version, temp
			memset(host,0,1000);
			memset(version,0,1000);
			memset(temp,0,1000);
			// write HTTP request message
			write(STDOUT_FILENO," - Message : ",15);
			write(STDOUT_FILENO,buf,len_out);
			write(STDOUT_FILENO,"\n",1);
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
			getcwd(current_path,10000); // get current working directory path
			if(t_len!=0)	// if exist relative path
				strcat(current_path,temp);	// add relative path to current working directory path
			ls_print(client_fd,host,temp,current_path);	// go to ls_print function
		}
		printf("[%d:%d] client was disconnected.\n",client_addr.sin_addr.s_addr,client_addr.sin_port);
		printf("\n\n");
		close(client_fd);	// close client descriptor
	}
	close(socket_fd);	// close socket descriptor
	return 0;
}

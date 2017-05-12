#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
int main(int argc, char **argv){
	DIR *dirp;
	struct dirent *dir;
	struct stat  sta;
	char arr_str[10000];		// all string     
	char w1[100]={};		// first word
	char w2[100]={};		// second word
	char c1,c2;			// first character, second character
	int len,a_len,l,l1,l2;		// words' length
	int i,j,s,c,k,k1,k2,s1,s2;	// integer variable(for for loop)
	int count,count_w;		// integer variable(word count)
	char print_arr[100]={};		// word to print 
	int aflag=0,lflag=0,flag;	// option
	int index1,index2;		// index
	int total=0,h_total=0;		// total
	char info[10000]={};		// information
	char path[10000]={};		// directory path
	char permission[1000][256];	// permission
	int linkcounter[1000]={};	// linkcounter
	char u_ID[1000][256];		// userID
	char g_ID[1000][256];		// group ID
	int capacity[1000]={};		// capacity
	char time[1000][256];		// time
	char c_time[1000];		// change time
	char *t_time;			// token time
	char t_path[1000];		// token path
	char tmp[256]={};		// swap string
	int swap_i=0,number;		// swap interger, for for loop
	char command[1000][256];	// command
	char command_p_c[1000][256];	// parent and current directory
	int ok=0,p_c_ok=1,a_r_ok=0;	// exist command
	int co_count=0,p_c_count=0;	// command counter
	int f1,f2=0,f3;			// integer variable to swap command
	int h1=0,h2=0,a_p=0;		// integer vairable
	int c_l1,c_l2,s_l;		// commands' length
	opterr=0;

	while((flag=getopt(argc,argv,"al"))!=-1){ // option select
		switch(flag){
		case 'a':	// ls -a
			aflag=1;
			break;
		case 'l':	// ls -l
			lflag=1;
			break;	
		case '?':	// if no option, end program
			printf("ls: invalid option -- '%c'\nTry 'ls --help' for more information.\n",optopt);
			return 0;
		}
	}
	for(i=optind;i<argc;i++){	// recieve path
		if(strcmp(argv[i],"..")!=0&&strcmp(argv[i],".")!=0&&argv[i][0]!='/'&&strcmp(argv[i],"~")!=0){	// no ~ & ..& .& absolute path
			strcpy(command[co_count],argv[i]);
			co_count++;
			p_c_ok=0;
		}
		else{	// ~ or .. or . or absolute path
			strcpy(command_p_c[p_c_count],argv[i]);
			p_c_count++;
		}
		ok=1;
	}
	if(p_c_ok==1)	// exist absolute or .. or . or ~
		a_p=p_c_count;
	// ---------------------------bubble sort
	if(p_c_count!=0){	// ~ or .. or . or absolute path
		for(k1=0;k1<p_c_count;k1++){
		for(k2=0;k2<p_c_count-1;k2++){
			c_l1=c_l2=0;	// initialize variables
			// count commands' length
			while(command_p_c[k2][c_l1]!='\0')
				c_l1++;
			while(command_p_c[k2+1][c_l2]!='\0')
				c_l2++;
			if(c_l1<c_l2) // compare commands' length
				l=c_l2;
			else
				l=c_l1;
			for(i=0;i<l;i++){ // compare commands' character
				if(command_p_c[k2][i]==command_p_c[k2+1][i]) // characters are same
					continue;
				else if(command_p_c[k2][i]>command_p_c[k2+1][i]){ // switch position(first command > second command)
					strcpy(tmp,command_p_c[k2]);
					strcpy(command_p_c[k2],command_p_c[k2+1]);
					strcpy(command_p_c[k2+1],tmp);
					break;
				}
				else	// first command < second command
					break;
			}
		}
		}
	}	
	while(1){ 	// start ls
	s_l=total=h_total=index1=index2=j=f2=a_len=count_w=0; // initialize variables	
	memset(arr_str,0,10000);	// initialize array
	for(i=0;i<1000;i++){	// initialize 2D array
		memset(permission[i],0,256);
		memset(u_ID[i],0,256);
		memset(g_ID[i],0,256);
		memset(time[i],0,256);
	}
	// open directory
	if(a_p==0)	// no absolute path
		dirp=opendir(".");
	else	// ~ or absolute path or .. or .
		dirp=opendir(command_p_c[h1]);
	if(dirp!=NULL){ // exist directory
	// read directory
	while((dir=readdir(dirp))!=NULL){	
		len=k=index2=number=k1=0;	// initialize variables
		memset(c_time,0,256);		// initialize string
		if(a_p==0)
			getcwd(path,10000);		// receive directory path
		else{
			if(p_c_count==0)	// no ~ & ..& .& absolute path
				strcpy(path,command[h1]);
			else{	// ~ or .. or . or absolute path
				memset(t_path,0,1000);
				strcpy(path,command_p_c[h1]);
				if(strcmp(command_p_c[h1],"..")==0){ // ..
					while(path[s_l]!='\0') // count path's length
						s_l++;
					for(k1=0;k1<s_l;k1++){ // find position of last /
						if(path[k1]=='/'&&path[k1+1]!='\0')
							k2=k1;
					}
					for(k1=0;k1<k2;k1++) // token path
						t_path[k1]=path[k1];
					strcpy(path,t_path);
				}
			}
		}
		strcpy(info,path);
		strcat(info,"/");
		strcat(info,dir->d_name);
		stat(info,&sta);
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
		strcpy(c_time,ctime(&sta.st_mtime)); // time
		// token time to make month day hour:minutes
		t_time=strtok(c_time," \t:");
		t_time=strtok(NULL," \t:");
		strcpy(time[index1],t_time);
		while(t_time!=NULL){
			t_time=strtok(NULL," \t:");
			if(number<3){
				if(number==2)
					strcat(time[index1],":");
				else
					strcat(time[index1]," ");
				strcat(time[index1],t_time);
			}
			number++;
		}
		total+=(int)sta.st_blocks/2; // overall total
		if(dir->d_name[0]=='.')	// hidden file total
			h_total+=(int)sta.st_blocks/2;
		while(dir->d_name[len]!='\0')	// calculate d_name's length
			len++;
		for(i=0;i<len;i++,k++)		// put file name in character array
			arr_str[i+j]=dir->d_name[k];
		arr_str[len+j]=' ';
		j=len+j+1;
		count_w++;	// count word
		a_len+=(len+1);	// arr_str's length
		index1++;
	}
	//---------------------Bubble sort-----------------
	if(ok==1){ // exist command
		if(a_p==0) // relative path
			f1=2;	
		else	// absolute path
			f1=1;
	}
	else	// no command
		f1=1;
	while(f2<f1){
	if(f2==0)	// f3 = files' counter
		f3=count_w;
	else		// f3 = commands' counter
		f3=co_count;
	for(k1=0;k1<f3;k1++){
	c=j=index1=0;	// initialize variables
	for(k2=0;k2<f3-1;k2++){
		s1=s2=0;	// initialize variables
		if(f2==0){	// receive file's name
		for(count=0;count<2;count++)	// find words to compare
		{
			for(i=j,s=0;i<a_len;i++,s++){
				if(arr_str[i]==' ')
					break;
				else{
					if(count==0){	// find first word and character
						c1=arr_str[j];
						w1[s]=arr_str[i];
					}
					else{	// find second word and character
						c2=arr_str[j];
						w2[s]=arr_str[i];
					}
				}
			}
			j=i+1;
		}
		}
		else{	// receive command's name
			strcpy(w1,command[k2]); // first command and character
			c1=w1[0];
			strcpy(w2,command[k2+1]); // second command and character
			c2=w2[0];
		}
		l1=l2=0;	// initialize variable
		// calculate words' length
		while(w1[l1]!='\0')
			l1++;	
		while(w2[l2]!='\0')
			l2++;
		// ---------------hidden file------------------
		if(c1=='.'){	// first word
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
		if(c2=='.'){	// second word
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
			if(f2==0){
			// change file's position
			for(i=c+l2+1,s=0;i<=c+l1+l2;i++,s++)
				arr_str[i]=w1[s];
			for(i=c,s=0;i<c+l2;i++,s++)
				arr_str[i]=w2[s];
			arr_str[c+l2]=' ';
			j-=(l1+1);
			// change permission's position
			strcpy(tmp,permission[index1]);
			strcpy(permission[index1],permission[index1+1]);
			strcpy(permission[index1+1],tmp);
			// change linkcounter's position
			swap_i=linkcounter[index1];
			linkcounter[index1]=linkcounter[index1+1];
			linkcounter[index1+1]=swap_i;
			// change user ID's position
			strcpy(tmp,u_ID[index1]);
			strcpy(u_ID[index1],u_ID[index1+1]);
			strcpy(u_ID[index1+1],tmp);
			// change group ID's position
			strcpy(tmp,g_ID[index1]);
			strcpy(g_ID[index1],g_ID[index1+1]);
			strcpy(g_ID[index1+1],tmp);
			// change capacity's position
			swap_i=capacity[index1];
			capacity[index1]=capacity[index1+1];
			capacity[index1+1]=swap_i;
			// change time's position
			strcpy(tmp,time[index1]);
			strcpy(time[index1],time[index1+1]);
			strcpy(time[index1+1],tmp);
			}
			else{
				// change command's position
				strcpy(tmp,command[number]);
				strcpy(command[number],command[number+1]);
				strcpy(command[number+1],tmp);
			}
		}
		else	// first word's character < second word's character
				j-=(l2+1);
		c=j;
		// initialize words
		for(i=0;i<l1;i++)
			w1[i]='\0';
		for(i=0;i<l2;i++)
			w2[i]='\0';
		index1++; // increase index1
	}
	}
		f2++;	// increase f2
	}	
	// --------------------------------print sort array
	// print directory path and total
	if(lflag==1){	// use option -l
		if(ok==0){	// no command
			printf("Directory path: %s\n",path);
			if(aflag==0)
				printf("total %d\n",total-h_total);
			else
				printf("total %d\n",total);
		}
		else{	// exist command
			if(a_p>0){ // exist absolute path
				printf("%s:\n",command_p_c[h1]);
				if(aflag==0)
					printf("total %d\n",total-h_total);
				else
					printf("total %d\n",total);
			}
		}
	}
	else{	// not use option -l
		if(a_p>0)
			printf("%s:\n",command_p_c[h1]);
	}	
	j=index1=0; // initialize variables(order of array)
	for(i=0;i<a_len;i++){
		if(arr_str[i]==' '){ // arr_str's character is blank
			if(aflag==0&&lflag==0){	// ls
				if(print_arr[0]!='.'){
					if(ok==1){ // exist command
						if(a_p==0){	// relative path
							for(number=0;number<co_count;number++){	// indirect path
								if(strcmp(command[number],print_arr)==0)
									printf("%s\n",print_arr);
							}
						}	
						else	// absolute path 
							printf("%s\n",print_arr);	
					}
					else	// no command
						printf("%s\n",print_arr);
				}
			}
			else if(aflag==1&&lflag==0){	// ls -a
				if(ok==1){	// exist command
					if(a_p==0){ // relative path
						for(number=0;number<co_count;number++){	// indirect path
							if(strcmp(command[number],print_arr)==0)
								printf("%s\n",print_arr);
						}
					}
					else	// absolute path
						printf("%s\n",print_arr);
				}
				else	// no command
					printf("%s\n",print_arr);
			}
			else if(aflag==0&&lflag==1){	// ls -l
				if(print_arr[0]!='.'){
					if(ok==1){	// exist command
						if(a_p==0){ // relative path
							for(number=0;number<co_count;number++){	// indirect path
								if(strcmp(command[number],print_arr)==0)
									printf("%s %d %s %s %d %s %s\n",permission[index1],linkcounter[index1],u_ID[index1],g_ID[index1],capacity[index1],time[index1],print_arr);
							}
						}
						else	// absolute path
							printf("%s %d %s %s %d %s %s\n",permission[index1],linkcounter[index1],u_ID[index1],g_ID[index1],capacity[index1],time[index1],print_arr);
					}
					else // no command
						printf("%s %d %s %s %d %s %s\n",permission[index1],linkcounter[index1],u_ID[index1],g_ID[index1],capacity[index1],time[index1],print_arr);
				}
			}
			else{	// ls -al
				if(ok==1){	// exist command
					if(a_p==0){ // absolute path
						for(number=0;number<co_count;number++){	// indirect path
								if(strcmp(command[number],print_arr)==0)
									printf("%s %d %s %s %d %s %s\n",permission[index1],linkcounter[index1],u_ID[index1],g_ID[index1],capacity[index1],time[index1],print_arr);
						}
					}
					else	// relative path
						printf("%s %d %s %s %d %s %s\n",permission[index1],linkcounter[index1],u_ID[index1],g_ID[index1],capacity[index1],time[index1],print_arr);
				}
				else	// co command
					printf("%s %d %s %s %d %s %s\n",permission[index1],linkcounter[index1],u_ID[index1],g_ID[index1],capacity[index1],time[index1],print_arr);
			}
			for(k=0;k<j;k++)	// initialize print_arr
				print_arr[k]='\0';
			j=0;	// initialize variable
			index1++;	// increase index1
		}
		else{	// arr_str's character isn't blank
			print_arr[j]=arr_str[i];
			j++;
		}
	}
	printf("\n");
	}
	else
		printf("%s : error\n",command_p_c[h1]);
	closedir(dirp);	// close directory
	h1++;	// increase variable
	// exit while statement
	if(a_p==0){	// no command
		if(p_c_count==0)	// no absolute command
			break;
		else{	// exist absolute command
			a_p=p_c_count;
			h1=0;
			continue;
		}
	}
	else{	// exist command 
		if(a_p==h1)	// if h1 = a_p, exit while statement
			break;
	}
	}
	return 0;
}

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
double rounding(float num){	// rounding function
	double ro=0,fl=0;
	int h;
	h=(int)(num*100);
	fl=(double)h/100;
	if((int)(num*100-h*10)<5)	// if num*100-h*10 is less than 5
		ro=fl;
	else	// if num*100-h*10 is more than 5
		ro=fl+0.01;
	return ro;	// return ro
}
void print_block(int block){	// print block function(use option -h)
	float h_block=0;
	if(block<1024)	// if block is less than 1024(K)
		printf("%dK ",block);
	else if(block<1024*1024){	// if block is less than 1024*1024(M)
		h_block=(float)block/1024;
		printf("%.1fM ",rounding(h_block));
	}
	else{	// if block is more than 1024*1024(G)
		h_block=(float)block/(1024*1024);
		printf("%.1fG ",rounding(h_block));
	}	
}
void print_h(int value){	// print function(use option -h)
	float h_value=0;
	if(value<1024)	// if value is less than 1024
		printf("%d ",value);
	else if(value<1024*1024){	// if value is less than 1024*1024(K)
		h_value=(float)value/1024;
		printf("%.1fK ",rounding(h_value));
	}
	else if(value<1024*1024*1024){	// if value is less than 1024*1024*1024(M)
		h_value=(float)value/(1024*1024);
		printf("%.1fM ",rounding(h_value));
	}
	else{	// if value is more than 1024*1024*1024(G)
		h_value=(float)value/(1024*1024*1024);
		printf("%.1fG ",rounding(h_value));
	}
}
int main(int argc, char **argv){
	DIR *dirp;
	struct dirent *dir;
	struct stat  sta;    
	char error[1000][256];				// error message
	char relative[1000][256];			// relative path
	char absolute[1000][256];			// absolue path
	char directory[1000][256];			// directory
	char fn_arr[1000][256];				// fnmatch
	int block[1000];				// block size
	char permission[1000][256];			// permission
	int linkcounter[1000];				// linkcounter
	char u_ID[1000][256];				// userID
	char g_ID[1000][256];				// group ID
	int capacity[1000];				// capacity
	int month[1000];				// month
	int day[1000];					// day
	int hour[1000];					// hour
	int minute[1000];				// minute
	char file[1000][256];				// file name
	char path[10000]={};				// directory path
	char info[10000]={};				// information
	char t_path[1000];				// token path
	char t_fn[1000];				// token wild card
	char tmp[256]={};				// swap string
	char w1[100]={};				// first word
	char w2[100]={};				// second word
	char w_file[1000][256];				// file when use wild card
	int information[1000];				// block, link counter, capacity, month, day, hour, minute
	char c1,c2;					// first character, second character
	int aflag=0,lflag=0,hflag=0,sflag=0,Sflag=0,flag;	// option
	int total=0,h_total=0;				// overall total and hidden file total
	int i_index,w_index,d_index,index1,index2;	// index
	int e_len=0,r_len=0,a_len=0,f_len=0;		// error's length, relative path's length, absolute path's length, fnmatch's length
	int l,l1,l2;					// words' length
	int i,j,s,k1,k2,s1,s2;				// integer variable(for for loop)
	int swap_i=0;					// swap interger
	int a_order=0,f_order=0,r_order=0;		// absolute path's order, wild card's order, relative path's order
	int w_end,f_end;				// for loop
	int fn=0;					// fnmatch variable
	int ok,f_ok,f_p_ok;				// check wild card and error 
	int d_count,read_d;				// directory count, read directory
	opterr=0;
	while((flag=getopt(argc,argv,"alhsS"))!=-1){ // option select
		switch(flag){
		case 'a':	// ls -a
			aflag=1;
			break;
		case 'l':	// ls -l
			lflag=1;
			break;	
		case 'h':	// ls -h
			hflag=1;
			break;
		case 's':	// ls -s
			sflag=1;
			break;
		case 'S':	// ls -S
			Sflag=1;
			break;
		case '?':	// if no option, end program
			printf("ls: invalid option -- '%c'\nTry 'ls --help' for more information.\n",optopt);
			return 0;
		}
	}
	for(i=0;i<1000;i++){	// initialize relative, absolute, fnmatch array
		memset(relative[i],0,256);
		memset(absolute[i],0,256);
		memset(fn_arr[i],0,256);
		memset(error[i],0,256);
	}
	for(i=optind;i<argc;i++){	// recieve path
		f_ok=0;
		for(j=0;argv[i][j]!='\0';j++){	// compare whether wild card or not
			if(argv[i][j]=='*'||argv[i][j]=='?'||argv[i][j]=='['){	// if wild card exist
				f_ok=1;
				break;
			}
		}
		if(f_ok==1)	// wild card
			strcpy(fn_arr[f_len++],argv[i]);
		else{
			if(strcmp(argv[i],"..")!=0&&strcmp(argv[i],".")!=0&&argv[i][0]!='/'&&strcmp(argv[i],"~")!=0)	// relataive path
				strcpy(relative[r_len++],argv[i]);
			else	// absolute path
				strcpy(absolute[a_len++],argv[i]);
		}
	}
	// ----------------------bubble sort(wild card)-----------------------------------------
	for(k1=0;k1<f_len;k1++){
		for(k2=0;k2<f_len-1;k2++){
			l1=l2=0;	// initialize variables
			// calculate wild cards' length
			while(fn_arr[k2][l1]!='\0')	// first wild card
				l1++;
			while(fn_arr[k2+1][l2]!='\0')	// second wild card
				l2++;
			// compare wild cards' length
			if(l1<l2) 	// first < second
				l=l2;
			else		// first > second
				l=l1;
			for(i=0;i<l;i++){ // compare wild cards' character
				if(fn_arr[k2][i]!=fn_arr[k2+1][i]){	// if first and second are different
					if(fn_arr[k2][i]>fn_arr[k2+1][i]){ // switch position(first > second)
						strcpy(tmp,fn_arr[k2]);
						strcpy(fn_arr[k2],fn_arr[k2+1]);
						strcpy(fn_arr[k2+1],tmp);	
					}
					break;
				}
			}
		}
	}
	// ---------------------------------------start ls----------------------------------------------------
	while(1){
		read_d=l=total=h_total=index1=index2=0; // initialize variables
		memset(block,0,1000);	// initialize 1D array(block)
		memset(linkcounter,0,1000); // initialize 1D array(link counter)
		memset(capacity,0,1000); // initialize 1D array(capacity)	
		memset(month,0,1000);	// initialize 1D array(month)
		memset(day,0,1000);	// initialize 1D array(day)
		memset(hour,0,1000);	// initialize 1D array(hour)
		memset(minute,0,1000);	// initialize 1D array(minute)
		for(i=0;i<1000;i++){	// initialize 2D array
			memset(permission[i],0,256);
			memset(u_ID[i],0,256);
			memset(g_ID[i],0,256);
			memset(file[i],0,256);
		}
		// ----------------------bubble sort(absolute path)-----------------------------------------
		for(k1=0;k1<a_len;k1++){
			for(k2=0;k2<a_len-1;k2++){
				l1=l2=0;	// initialize variables
				// calculate abolute paths' length
				while(absolute[k2][l1]!='\0')	// first absolute path
					l1++;
				while(absolute[k2+1][l2]!='\0')	// second absolute path
					l2++;
				// compare absolute paths' length
				if(l1<l2) 	// first < second
					l=l2;
				else		// first > second
					l=l1;
				for(i=0;i<l;i++){ // compare absolute paths' character
					if(absolute[k2][i]!=absolute[k2+1][i]){	// if first and second are different
						if(absolute[k2][i]>absolute[k2+1][i]){ // switch position(first > second)
							strcpy(tmp,absolute[k2]);
							strcpy(absolute[k2],absolute[k2+1]);
							strcpy(absolute[k2+1],tmp);	
						}
						break;
					}
				}
			}
		}	
		while(1){	// open directory
			ok=i_index=w_index=d_index=f_p_ok=d_count=0;	// initialize variables
			memset(information,0,1000);	// initialize 1D array(information)
			for(i=0;i<1000;i++){	// initialize 2D array
				memset(directory[i],0,256);
				memset(w_file[i],0,256);
			}
			if(r_len!=0)	// if relative path exist
				dirp=opendir("."); 		// open current directory
			else if(f_len!=0){	// if wild card exist
				if(fn_arr[f_order][0]=='/'){
					memset(t_fn,0,1000);
					for(i=0;fn_arr[f_order][i+1]!='*'&&fn_arr[f_order][i+1]!='?'&&fn_arr[f_order][i+1]!='[';i++)	// find absolute path except * or ? or [
						t_fn[i]=fn_arr[f_order][i];
					dirp=opendir(t_fn);	// open t_fn's directory
					f_p_ok=1;
				}
				else
					dirp=opendir("."); // open current directory
			}
			else if(a_len!=0)	// if absolute path exist
				dirp=opendir(absolute[a_order]);	// open absolute path's directory
			else	// if path and wild card don't exist
				dirp=opendir(".");
			if(dirp!=NULL){
				read_d=1; // can read directory
				while((dir=readdir(dirp))!=NULL){	
					index2=w_end=0;	// initialize variables
					getcwd(path,10000);	// receive  current directory path
					if(f_p_ok==1)	// if wild card is absolute path
						strcpy(path,t_fn);		
					if(r_len==0&&f_len==0&&a_len!=0){	// if absolute path exist
						memset(t_path,0,1000);	// initialize t_path
						strcpy(path,absolute[a_order]);
						if(strcmp(absolute[a_order],"..")==0){ // if absolute path is ..
							for(i=0;path[i]!='\0';i++){ // find position of last '/'
								if(path[i]=='/'&&path[i+1]!='\0')
									j=i;
							}
							for(i=0;i<j;i++) // token path
								t_path[i]=path[i];
							strcpy(path,t_path);
						}
					}	
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
					block[index1]=(int)sta.st_blocks/2;	// the number of 1K blocks
					total+=(int)sta.st_blocks/2; // overall total
					if(dir->d_name[0]=='.')	// hidden file total
						h_total+=(int)sta.st_blocks/2;
					if(r_len!=0){	// if relative path exist
						if(strcmp(relative[r_order],dir->d_name)==0){	// if relative path = file's name
								index1++;
								ok=1;	// check whether file exist or not
						}
					}
					else if(f_len!=0){	// if wild card exist
						if(file[index1][0]!='.'){ // if file isn't hidden file
							if(f_p_ok==0)	// if wild card is relative path
								fn=fnmatch(fn_arr[f_order],dir->d_name,0);	
							else	// if wild card is absolute path
								fn=fnmatch(fn_arr[f_order],info,0);
							if(fn==0){	// match
								ok=1;	// check whether wild card exist or not
								if(permission[index1][0]=='d'){ // if file is directory
									strcpy(directory[d_index++],info);
									if(f_p_ok==0)	// if wild card is relative path
										strcpy(w_file[w_index++],dir->d_name);
									else	// if wild card is absolute path
										strcpy(w_file[w_index++],info);
									strcpy(w_file[w_index++],permission[index1]); // save permission
									strcpy(w_file[w_index++],u_ID[index1]);	// save user ID
									strcpy(w_file[w_index++],g_ID[index1]);	// save group ID
									information[i_index++]=block[index1];	// save block size
									information[i_index++]=linkcounter[index1];	// save linkcounter
									information[i_index++]=capacity[index1];	// save capacity
									information[i_index++]=month[index1];	// save month
									information[i_index++]=day[index1];	// save day
									information[i_index++]=hour[index1];	// save hour
									information[i_index++]=minute[index1];	// save minute
									d_count++;
								}
								else{	// if file is file
									if(f_p_ok==0)	// if wild card is relative path
										strcpy(file[index1++],dir->d_name);
									else	// if wild card is absolute path
										strcpy(file[index1++],info);
								}
							}		
						}	
					}
					else	// if fnmatch doesn't exist
						index1++;
				}
			}	
			if(r_len==0){
				f_order++;	// increase wild card's order
				if(f_len!=0){	// not match
					if(ok==0)	// if wild card and info are different
						strcpy(error[e_len++],fn_arr[f_order-1]);
					if(d_count>1){	// directory count is more than 1
						for(i=0;i<d_index;i++)	// keep directory in absolute
							strcpy(absolute[a_len++],directory[i]);
					}
					else{	// directory count is less than 2
						if(strcmp(w_file[0],"\0")!=0){ // if w_file isn't null
							strcpy(file[index1],w_file[0]);	// save file 
							strcpy(permission[index1],w_file[1]);	// save permission
							strcpy(u_ID[index1],w_file[2]);	// save user ID
							strcpy(g_ID[index1],w_file[3]);	// save group ID
							block[index1]=information[0];	// save block size
							linkcounter[index1]=information[1];	// save linkcounter
							capacity[index1]=information[2];	// save capacity
							month[index1]=information[3];	// save month
							day[index1]=information[4];	// save day
							hour[index1]=information[5];	// save hour
							minute[index1++]=information[6];	// save minute		 		
						}
					}
				}
				if(f_len==0||f_len==f_order)	// if wild card doesn't exist or wild card's length and order are same
					break;
			}
			else{
				if(ok==0)	// if relative path and file name are different
					strcpy(error[e_len++],relative[r_order]);
				r_order++;
				if(r_len==r_order){	// if wild card exist	
					if(f_len==0)	// if wild card doesn't exist
						break;	
					else	// if wild card exist
						r_len=0;	// intialize variable(relative paths' length)
				}			
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
			//---------------------Bubble sort(capacity)(ls -S)---------------------
			if(Sflag==1){			
				for(k1=0;k1<index1;k1++){
					for(k2=0;k2<index1-1;k2++){
						if(capacity[k2]<capacity[k2+1]){	// first > second
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
					}
				}
			}
			// --------------------------------print sort array
			// print directory path and total	
			if(r_len==0&&f_len==0){
				if(lflag==1){	// use option -l
					if(a_len==0)	// if absolute path doesn't exist
						printf("Directory path: %s\n",path);
					else	// if absolute path exists 
						printf("%s:\n",absolute[a_order]);
					if(aflag==0)	// if not use option -a
							total=total-h_total;
					if(hflag==1){	// exist option -h
						printf("total ");
						print_block(total);	// go to print_block function				
					}
					else	// not exist option -h
						printf("total %d",total);
					printf("\n");
				}
				else{	// not use option -l
					if(a_len!=0)
						printf("%s:\n",absolute[a_order]);
				}
			}	
			// print information
			for(i=0;i<index1;i++){
				if(aflag==0||f_len!=0){		// no option -a(except hidden file)
					if(file[i][0]!='.'){	// if file isn't hidden file
						if(sflag==1){	// ls -s
							if(hflag==0)	// not use option -h
								printf("%d ",block[i]);
							else	// use option -h
								print_block(block[i]);	// go to print_block function
						}
						if(lflag==1){	// ls -l
							printf("%s ",permission[i]);
							if(hflag==0) // not use option -h
								printf("%d ",linkcounter[i]);
							else	// use option -h
								print_h(linkcounter[i]); // go to print_h function
							printf("%s %s ",u_ID[i],g_ID[i]);
							if(hflag==0) 	// not use option -h
								printf("%d ",capacity[i]);
							else	// use option -h
								print_h(capacity[i]);	// go to print_h function
							printf("%d %d %d:%d ",month[i],day[i],hour[i],minute[i]);
						}
						printf("%s\n",file[i]);
					}
				}
				else{
					if(sflag==1){	// ls -s
						if(hflag==0)	// not use option -h
							printf("%d ",block[i]);
						else	// use option -h
							print_block(block[i]);	// go to print_block function
					}
					if(lflag==1){
						printf("%s ",permission[i]);
						if(hflag==0)	// not use option -h
							printf("%d ",linkcounter[i]);
						else	// use option -h
							print_h(linkcounter[i]);	// go to print_h function
						printf("%s %s ",u_ID[i],g_ID[i]);
						if(hflag==0)	// not use option -h
							printf("%d ",capacity[i]);
						else	// use option -h
							print_h(capacity[i]);	// go to print_h function
						printf("%d %d %d:%d ",month[i],day[i],hour[i],minute[i]);
					}
					printf("%s\n",file[i]);
				}
			}
		}
		else	// no directory path
			strcpy(error[e_len++],absolute[a_order]);
		printf("\n");
		closedir(dirp);	// close directory
		a_order++;	// increase variable(absolute path's order)
		// exit while statement
		if(f_len==0){	// if wild card doesn't exist
			if(r_len==0){	// if relative path doesn't exist
				if(a_len==0||a_order==a_len)	// absolute path doesn't exist or done absolute path
					break;
			}
			else{	// if relative path exists
				if(a_len==0)	// absolute path doesn't exist
					break;
				else
					a_order=r_len=0; // initialize relative path's length & absolute path's order		
			}
		}
		else{	// if wild card exist
			if(a_len==0)	// if absolute path doesn't exist
				break;
			else
				a_order=f_len=0; // initialize absolute path's length & wild card's length
		}
	}
	for(i=0;i<e_len;i++)	// print error message
		printf("ls: cannot access %s: No such file or directory\n",error[i]);
	return 0;
}

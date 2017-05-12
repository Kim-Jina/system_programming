#include <stdio.h>
#include <dirent.h>
int main(){
	DIR *dirp;
	struct dirent *dir;
	char arr_str[10000]={};		// all string     
	char w1[100]={};		// first word
	char w2[100]={};		// second word
	char c1,c2;			// first character, second character
	int len,a_len=0,l,l1,l2;	// words' length
	int i,j=0,s,c,k,k1,k2,s1,s2;	// integer variable(for for loop)
	int count,count_w=0;		// integer variable(word count) 
	
	dirp=opendir(".");	// open directory
	
	while((dir=readdir(dirp))!=NULL){	// read directory
		len=k=0;	// initialize variables
		while(dir->d_name[len]!='\0')	// calculate d_name's length
			len++;
		for(i=0;i<len;i++,k++)		// put file name in character array
			arr_str[i+j]=dir->d_name[k];
		arr_str[len+j]=' ';
		j=len+j+1;
		count_w++;	// count word
		a_len+=(len+1);	// arr_str's length
	}
	//---------------------Bubble sort-----------------
	for(k1=0;k1<count_w;k1++){
	c=j=0;	// initialize variables
	for(k2=0;k2<count_w-1;k2++){
		s1=s2=0;	// initialize variables
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
			// change position
			for(i=c+l2+1,s=0;i<=c+l1+l2;i++,s++)
				arr_str[i]=w1[s];
			for(i=c,s=0;i<c+l2;i++,s++)
				arr_str[i]=w2[s];
			arr_str[c+l2]=' ';
			j-=(l1+1);
		}
		else	// first word's character < second word's character
			j-=(l2+1);
		c=j;
		// initialize words
		for(i=0;i<l1;i++)
			w1[i]='\0';
		for(i=0;i<l2;i++)
			w2[i]='\0';
	}
	}
	// --------------------------------print sort array
	for(i=0;i<a_len;i++){
		if(arr_str[i]==' ')
			printf("\n");
		else
			printf("%c",arr_str[i]);
	}
	closedir(dirp);	// close directory
	return 0;
}

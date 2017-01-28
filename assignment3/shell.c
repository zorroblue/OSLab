#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>

#define MAX_QUERY_SIZE 1024
#define MAX_PARAMS 100

int main()
{
    char present_dir[1024]; //assume the length wouldn't exceed 1024
    int exit_flag=0; //we use this flag for child processes to stop executing the infinite loop more than once
    while(1)
    {
	//if child process executes this loop again, break
	if(exit_flag==1)
	{
		exit_flag=0;
		exit(1);
	}
         if (getcwd(present_dir, sizeof(present_dir)) == NULL)
         {
            perror("getcwd() error");
            exit(-1);
         }
        printf("%s> ",present_dir);
        char query[MAX_QUERY_SIZE];
        //scanf("%s",query);
        fgets (query, MAX_QUERY_SIZE-1, stdin);
        if(query==NULL)
		break;
	char *pch = strtok(query," ");
        char *newline = strchr(pch,'\n');
        if(newline)
            *newline = 0;
        char *command = pch;
        char **args = (char **)malloc(MAX_PARAMS*(sizeof(char *)));
        
	//flag for background
	int background=0;
	//strings for input and output file
	char *inputfile=NULL,*outputfile=NULL;
	int inputflag=0,outputflag=0,ioflag=0; //if < and > have been read
	//ioflag set to 1 if there is any IO redirection in the command
	
	//flag for pipe
	int pipeflag=0;
	int no_args=0;
        args[no_args++] = command;
	int no_pipes=0;
	while(pch!=NULL)
        {
            //printf("%s\n",pch);
            //tokenise the query arguments
            pch = strtok(NULL," ");
            //take care of empty/null strings
            if(pch==NULL || strlen(pch)==0)
                break;

            //remove newline character if present
            char *newline = strchr(pch,'\n');
            if(newline)
                *newline = 0;
            // add to argument list
	    if(inputflag==1)
	    {
		    inputfile = pch;
		    inputflag=2;
	    }
	    if(outputflag==1)
	    {
		    printf("Output file is %s\n",pch);
		    outputfile = pch;
		    outputflag = 2;
	    }
	    if(strcmp(pch,"&")==0)
	    {
		    background=1;
		    break;
	    }
	    if(strcmp(pch,"<")==0)
	    {
		    inputflag=1;
		    ioflag=1;
	    }
	    if(strcmp(pch,">")==0)
	    {
		    printf("> found\n\n");
		    outputflag=1;
		    ioflag=1;
	    }
	    
	    if(strcmp(pch, "|")==0)
	    {
		    pipeflag=1;
		    no_pipes++;
	    }

	    args[no_args++] = pch;
        }

	//if background process
	
	int id1;
	if(background==1)
	{
		id1=fork();
		if(id1==-1)
		{
			perror("The background process couldn't be created ");
			continue;
		}
		else if(id1==0) //child process
		{
			exit_flag=1;
			setpgid(0,0); //put the child in a new process group

		}
		else //parent process
		{
			continue;
		}
	}
	//printf("The io flag = %d %d %d\n",ioflag,inputflag,outputflag);
	
	
	int i;

	// if there are pipes
	if(pipeflag==1)
	{
		char ***ch;
		ch = (char ***)malloc(sizeof(char **)*(no_pipes+1));
		int j=0;
		int k=0;
		for(i=0;i<no_args;i++)
		{
			int j=i;
			while(i<no_args && args[i]!=NULL && !strcmp(args[i],"|")==0)
				i++;
			ch[k]=(char **)malloc(sizeof(char *)*(i-j+1));
			int temp=j;
	//		printf("%d\n",k);
			while(temp<i)
			{
				ch[k][temp-j]=args[temp];
	//			printf("%s ",args[temp]);
				temp++;
			}
			k++;
	//		printf("\n");
		}

		int p[2],pid,fd_in;
		//inspired from StackOverflow
		while(ch!=NULL && *ch!=NULL)
		{
			pipe(p);
			pid = fork();
			if(pid == -1)
			{
				perror("Can't create child process ");
				exit(-1);
			}
			else if(pid == 0)
			{
				//child process
				dup2(fd_in,0);
				if(*(ch+1)!=NULL)
					dup2(p[1],1);
				close(p[0]);
				int status=execvp((*ch)[0],*ch);
				exit(-1);
				/*if(status == -1)
				{
					perror("Couldn't execute ");
					exit(-1);
				}
				else
				{
					exit(1);
				}*/
			}
			else
			{
				//parent process
				wait(NULL);
				close(p[1]);
				fd_in = p[0];
				ch++;
			}
		}
		continue;
	}


	//check the command
	
	if(strcmp(command,"pwd")==0)
	{
		//print current working directory
		printf("%s\n",present_dir);
	}
	else if(strcmp(command,"cd")==0)
	{
		//change directory
		if(no_args<=1)
		{
			printf("No directory specified\n");
			continue;
		}	
		int status = chdir(args[1]);
		if(status!=0)
		{
			//failure
			perror("Couldn't change directory ");
		}
	}
	else if(strcmp(command,"mkdir")==0)
	{
		//create new dir
		if(no_args<=1)
		{
			printf("No directory name specified\n");
			continue;
		}
		//give read/write/search permissions for owner and group and read/search permissions for others
		int  status = mkdir(args[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if(status!=0)
		{
			perror("Couldn't create directory ");
			continue;
	}
	}
	else if(strcmp(command,"rmdir")==0)
	{
		if(no_args<=1)
		{
			perror("No directory specified\n");
			continue;
		}
		int status = rmdir(args[1]);
		if(status==-1)
		{
			perror("Couldn't delete directory ");
		}

	}
	else if(strcmp(command,"ls")==0)
	{
		
		if(no_args>1 && strcmp(args[1],"-l")==0)
		{
			 DIR *dp;
			 struct dirent *sd; 
			 dp=opendir(present_dir); 		  
			 while((sd=readdir(dp))!=NULL)
			 {
				 char temp[1024];
				 strcpy(temp,present_dir);
				 strcat(temp,"/");
				 strcat(temp,sd->d_name);
				 struct stat status;
				 int stat_signal;
				 if((stat_signal=stat(temp,&status))==-1)
				 {
					 perror("stat failed");
					 break;
				 }
				 else
				 {
				    if(strcmp(sd->d_name,".")==0 || strcmp(sd->d_name,"..") == 0)
				        continue;
					printf("%s%s%s%s%s%s%s%s%s%s",(S_ISDIR(status.st_mode)) ? "d" : "-", (status.st_mode & S_IRUSR) ? "r" : "-", (status.st_mode & S_IWUSR) ? "w" : "-", (status.st_mode & S_IXUSR) ? "x" : "-",(status.st_mode & S_IRGRP) ? "r" : "-",(status.st_mode & S_IWGRP) ? "w" : "-",(status.st_mode & S_IXGRP) ? "x" : "-",(status.st_mode & S_IROTH) ? "r" : "-",(status.st_mode & S_IWOTH) ? "w" : "-",(status.st_mode & S_IXOTH) ? "x" : "-");
					char timestr[102];
					strftime(timestr, 100, "%b %d %H:%M", localtime(&(status.st_ctime)));
					printf(" %s ",timestr);
					struct passwd *pw = getpwuid(status.st_uid);
                    struct group  *gr = getgrgid(status.st_gid);
				    
				    printf("%d %s %s %d %s\n",(int)status.st_nlink,(pw!=0)?pw->pw_name:"-",(gr!=0)?gr->gr_name:"-",(int)status.st_size,sd->d_name);

				 }

				 
			 }
			 printf("\n");
			 closedir(dp);
			 
		}
		else //do a default ls
		{
			DIR *dp;
			 struct dirent *sd; 
			 dp=opendir(present_dir); 		  
			 while((sd=readdir(dp))!=NULL)
			 {
				 printf("%s\t",sd->d_name);
			 }
			 printf("\n");
			 closedir(dp);
		}
    
	}
	else if(strcmp(command,"cp")==0)
	{
		if(no_args<3)
		{
			printf("cp requires 2 arguments. Usage: cp <filename1> <filename2>\n ");
			continue;
		}
		else
		{
			FILE *fp1,*fp2;
//			printf("Args[1] = %s\n",args[1]);
			fp1 = fopen(args[1],"rb");
			if(fp1== NULL)
			{
				perror("File 1 is not present");
			}
			
			if(access(args[2],F_OK) !=-1)
			{
				//check for modified time
				struct stat status1,status2;
				int fstat2= stat(args[2],&status2);
				int fstat1 = stat(args[1],&status1);
				if(fstat1 == -1 || fstat2 == -1)
				{
					perror("stat failed");
					continue;
				}
				else
				{
					if( status1.st_ctime >= status2.st_ctime)
					{
						printf("Modified file2 before file1. So system can't copy\n ");
						continue;
					}

				}
			}
			//copy the file1 to file2
			fp2 = fopen(args[2],"wb");
			//not essentially needed, but including for an extra layer of safety. 
			if(fp2 ==NULL)
			{
				perror("File 2 :");
				continue;
			}
			char buffer[1024];
			size_t incount = fread(buffer,sizeof(char),1024,fp1);
			while(!feof(fp1))
			{
				fwrite(buffer,sizeof(char),incount,fp2);
				incount = fread(buffer,sizeof(char),1024,fp1);
			}				
			fwrite(buffer,sizeof(char),incount,fp2);
			fclose(fp1);
			fclose(fp2);
		}
	}
	else if(strcmp(command,"exit") == 0)
	{
		exit(1);
	}
	else
	{
		//treat as execution of an executable
		//create a new child process 
		int id = fork();
		if(id==-1)
		{
			perror("Error in executing");
			continue;
		}
		else if(id==0)
		{
			//child process
			//if IO redirection is involved
			if(ioflag==1)
			{
		        int in,out;
		        if(inputflag==2)
		        {
			        in = open(inputfile,O_RDONLY);
			        if(in==-1)
			        {
				        perror("Error opening input file ");
				        continue;
			        }
		        }
		        if(outputflag==2)
		        {
			        printf("The Output file is %s\n",outputfile);
			        out = open(outputfile,O_WRONLY | O_CREAT,0666);
			        if(out==-1)
			        {
				        perror("Error opening output file ");
				        close(in); //close the input file
				        continue;
			        }
		        }
		        if(inputflag==2)
		        {
			        dup2(in,STDIN_FILENO);
			        close(in);
		        }
		        if(outputflag==2)
		        {
			        dup2(out,STDOUT_FILENO);
			        close(out);
		        }
			}
			
			int status = execvp(args[0],args);
			printf("here %d\n",status);
			if(status!=256)
				perror("Error in executing ");
			exit(status);
		}
		else
		{
			//parent process
			int status=0;
			int child_status = waitpid(id,&status,0);
		       //wait till all processes are done
			//printf("stt : %d %d\n",child_status,status);
			
		}
	}
		
    }
}



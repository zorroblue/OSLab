#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MSGSIZE 100

int main(int argc, char **argv)
{
	if(argc!=3) // if the number of arguments is not 3, then there is an error
	{
		printf("Usage : ./a.out filename1 filename2");
		exit(-1);
	}
	
	//get filenames
	char *file1 = argv[1];
	char *file2 = argv[2];
	
	int fid1 = open(file1,O_RDONLY); // open file1 with read only mode
	int fid2 = open(file2,O_CREAT | O_WRONLY | O_TRUNC,S_IRUSR | S_IWUSR); //open file2 , create if not exists, and give permissions to owner and group to edit the file
//	printf("%d %d\n",fid1,fid2);
	if(fid1 == -1 || fid2 == -1)
	{
		perror("There is a problem opening the files");
		exit(-1);
	}
	//the files can be used now
	
	int fd1[2],fd2[2]; //fd1 pipe moves the bytes and fd2 pipe takes care of  the status

    //make two pipes, one for handling data and the other for status
	pipe(fd1);
	pipe(fd2);
	
	int val=0; // used for deciding
	//fork a child process

	int id = fork();
	
	if(id == -1)
	{
		perror("Couldn't create child process");
		exit(-1);
	}
	if(id==0)
	{
		//child process
		close(fd1[1]);
		close(fd2[0]);
		char buff[MSGSIZE+1];
		int val=0;
		while(1)
		{
			int readstatus = read(fd1[0],buff,MSGSIZE);
			if(readstatus<=0)
			{
				val=-1;
				write(fd2[1],&val,sizeof(val));
				perror("Exiting.");
				exit(-1);
			}
			else
			{
				val=0;
				write(fid2,buff,readstatus);
				
				if(readstatus<100)
				{
					write(fd2[1],&val,sizeof(val));
					printf("file copied successfully\n");
					exit(0);
				}
				else
					write(fd2[1],&val,sizeof(val));
			}
		}
		close(fd1[0]);
		close(fd2[1]);
	}
	else
	{
		//parent process
		close(fd1[0]); //the parent will write to pipe1
		close(fd2[1]); //the parent will read from pipe2
		ssize_t nbytes,bufsize=MSGSIZE;
		char buff[MSGSIZE+2];
		while((nbytes=read(fid1,buff,MSGSIZE))>0)
		{
			//write to pipe1
			write(fd1[1],buff,nbytes);
			//wait till acknowledgment
			char status[10]; 
			read(fd2[0],&val,sizeof(val)); //maximum for status
			if(val==0)
			{
				if(nbytes<MSGSIZE)
					exit(1);
				else
					continue;
			}
			else if(val==-1)
			{
				exit(-1);
			}
		}
		close(fd1[1]);
		close(fd2[0]);	
	}
	return 0;
}

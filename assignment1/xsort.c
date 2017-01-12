#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv)
{
	if(argc!=2)
	{
		printf("Usage : ./xsort <filename> \n");
		exit(-1);
	}
	else
	{
		//make a new child process to sort the process	
		int id = fork();
		if(id==0)
		{
			char *xtermargv[] = {"xterm","-hold","-e","./sort1",argv[1]};
			execvp("xterm",xtermargv);
			exit(1);
		}
	}
	exit(1);
}
			

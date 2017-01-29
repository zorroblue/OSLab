#include <stdio.h>
#include <stdlib.h>

int main()
{
	int id=fork();
	if(id==0) //child process
	{
		char *args[] = {"./shell",NULL};
		int status = execvp("./shell",args);
		if(status==-1)
		{
			perror("Couldn't execute shell ");
			exit(-1);
		}
	}
	else
	{
		wait(NULL);
	}
}

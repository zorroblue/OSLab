#include<stdio.h>
#include<stdlib.h>
// Do we assume the file is in the right format?
// Do we assume that there is an upper limit on the number of numbers in that file and it is 100?

#define MAX_SIZE 100

int first_pid; // the process id of the first process 


//linear search for x from a[begin] to a[end]
//return 1 if found else 0
int linear_search(int a[],int x,int begin,int end)
{
	int i;
	for(i=begin;i<=end;i++)
	{
		if(a[i]==x)
			return 1;
	}
	return 0;
}


void recurse_find(int a[], int x, int begin, int end)
{
	if(end-begin<=9) //size<=10
	{
		int answer = linear_search(a,x,begin,end);
		//corner case if the number of values is less than 10 in the beginning itself
		if(getpid() != first_pid)
			exit(answer);
		else
		{
			if(answer==256)
			
				printf("found\n");
			else
			{
				printf("not found\n");
			}
		}	
	}
	else
	{
		//create child processes
		int mid =(begin+end)/2;
		int id1 = fork();
		int id2 = fork();

		int result1=0,result2=0; //the results of the two processes
		if(id1==0)
		{
			//child process 1
			recurse_find(a,x,begin,mid);
		}
		if(id2==0)
		{
			//child process 2
			recurse_find(a,x,mid+1,end);
		}
		if(id1!=0 && id2!=0)
		{
			//parent process
			int status1=0,status2=0;
			waitpid(id1,&status1,0); // wait till the child processes are done
			waitpid(id2,&status2,0);

			//merge the solutions
			//status1 and status2 hold the values of the 2 children

			if(getpid() == first_pid) // this is the first process
			{
				if(status1==256 || status2==256)
					printf("found\n");
				else
					printf("not found\n");
			}
			else
			{
				exit(status1==256 || status2==256);
			}
		}
	}
}
			

int main()
{
	//input the filename
	printf("Enter the filename : ");
	char filename[20]; //assume the general filename size is 20 at max
	scanf("%s",filename);
	
	//open the file for reading
	FILE *fp = fopen(filename,"r");
	if(fp==NULL)
	{
		printf("The file does not exist!\n");
		exit(-1);
	}
	
	//the file is valid
	int a[MAX_SIZE];
	int i=1;

	fscanf(fp, "%d" , &a[0]);
	while(!feof(fp))
	{
		fscanf(fp, "%d",&a[i]);
		i++;
	}
	fclose(fp);

	int size = i-1;
	//read k
	int k;

	first_pid = getpid(); // get the process id of the first process

	while(1)
	{
		printf("Enter k : ");
		scanf("%d",&k);
		if(k>0)
		{
			recurse_find(a,k,0,size-1);
		}
		else
		{
			break;
		}
	}
	return 0;
}

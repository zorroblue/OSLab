#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define BUFFER_SIZE 20
#define NUMBERS 50

#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing the V(s) operation */

int main()
{
	int shmid,shmid_sum,status;
	//get a shared memory of an array of BUFFER_SIZE integers
	shmid = shmget(IPC_PRIVATE, (BUFFER_SIZE)*sizeof(int), 0777|IPC_CREAT);
	shmid_sum = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT); // for SUM
	int shmid_start = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT); // for start
	int shmid_end = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT); // for end
	


	// initialise start ,sum and end
	int *a = (int *) shmat(shmid, 0, 0);
	int *start = (int *) shmat(shmid_start, 0, 0);
	*start = 0;
	int *end = (int *) shmat(shmid_end, 0, 0);
	*end = 0;
	int *SUM = (int *) shmat(shmid_sum, 0, 0);
	*SUM = 0;
	
	shmdt(a);
	shmdt(start);
	//shmdt(read_no);
	shmdt(end);
	shmdt(SUM);

	 // empty and full conditions are taken care of by the semaphores

	int m,n;
	//input m and n
	printf("Enter the number of producers and consumers : ");
	scanf("%d %d",&m,&n);
	
	int i,j;

	//define the semaphores needed
	int mutex,full,empty;	
	mutex = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	full = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	empty = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	
	//set values for each semaphore
	semctl(mutex, 0, SETVAL, 1); //binary semaphore - access or not
	semctl(empty, 0, SETVAL, BUFFER_SIZE); // number of empty locations
	semctl(full, 0, SETVAL, 0); // number of full locations

    //printf("Values mutex= %d empty=%d full=%d\n",mutex,empty,full);
	struct sembuf pop,vop; //these are for wait and signal of semaphores
	pop.sem_num = 0;
	vop.sem_num = 0;
	pop.sem_flg = 0;
	vop.sem_flg = 0;
	pop.sem_op = -1; //decrement when P is executed
	vop.sem_op = 1; //increment when V is executed



	int id;

	//create two processes to do the producer and consumer

	int consumer_id,producer_id;
	producer_id = fork();
	if(producer_id==0)
	{
		//as the shared memory has already been accessed by the main process , we only need to detach our memory now
		// fork m producer processes
		for(i=0;i<m;i++)
		{
			id = fork();
			if(id==0)
			{
				int *a = (int *) shmat(shmid, 0, 0);
				int *start = (int *) shmat(shmid_start, 0, 0);
				int *end = (int *) shmat(shmid_end, 0, 0);
				int *SUM = (int *) shmat(shmid_sum, 0, 0);
			
				//write NUMBERS numbers to buffer
				for(j=1;j<=NUMBERS;j++)
				{
					P(empty);
					P(mutex);
					//write to buffer
					a[*end] = j;
					*end=(*end+1)%BUFFER_SIZE;
					//printf("Producer %d wrote %d start= %d end=%d\n",i,j,*start,*end);
					V(mutex);
					V(full);
				}
				
				shmdt(a);
				shmdt(start);
				shmdt(end);
				shmdt(SUM);
				//exit after done
				//printf("done!!\n");
				
				exit(1);
			} 
		}
		int wpid;
		while ((wpid = wait(&status)) > 0);
		//wait(NULL);
	}	
	else
	{
	
	//fork n consumer processes
	consumer_id = fork();
	if(consumer_id==0)
	{
		for(i=0;i<n;i++)
		{
			int id1 = fork();
			if(id1==0)
			{

				int count=0;
				while(1)
				{	
					int flag=1,l;
					int *a = (int *) shmat(shmid, 0, 0);
					int *start = (int *) shmat(shmid_start, 0, 0);
					int *end = (int *) shmat(shmid_end, 0, 0);
					int *SUM = (int *) shmat(shmid_sum, 0, 0);
					P(full);
					P(mutex);
					//read from buffer
					*SUM = *SUM + a[*start];
					*start = (*start+1)%BUFFER_SIZE;
					//printf("Read %d\n",a[*start]);
					
					V(mutex);
					V(empty);

					//printf("sum=%d\n",SUM[0]);
					int total = m*25*51;
					int temp_sum = SUM[0];
					shmdt(a);
					shmdt(start);
					//shmdt(read_no);
					shmdt(end);
					shmdt(SUM);
						//V(full);
					if(temp_sum >= total)
					{					
						exit(1);
					}	
				}
			}
		}
		//int wpid;
		//while ((wpid = wait(&status)) > 0);
		wait(NULL);
	}
	else
	{
		//wait till all processes are done
		int wpid;
		while ((wpid = wait(&status)) > 0);
		int *SUM = (int *) shmat(shmid_sum, 0, 0);
		printf("The sum is %d\n",*SUM);
		
		semctl(mutex, 0, IPC_RMID, 0);
		semctl(full, 0, IPC_RMID, 0);
		semctl(empty, 0, IPC_RMID, 0);


		

		shmctl(shmid, IPC_RMID, 0);
		shmctl(shmid_start, IPC_RMID, 0);
		shmctl(shmid_end, IPC_RMID, 0);
		//shmctl(shmid_read, IPC_RMID, 0);
		shmctl(shmid_sum, IPC_RMID, 0);
		}
	}

}
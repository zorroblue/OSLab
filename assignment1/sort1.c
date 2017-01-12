#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1001

void merge(int a[],int begin,int end)
{
	int c[end-begin+1];
	int mid = (begin+end)/2;
	int i=begin,j=mid+1,k=0;
	while(i<=mid && j<=end)
	{
		if(a[i]<=a[j])
		{
			c[k]=a[i];
			i++;
			k++;
		}
		else
		{
			c[k]=a[j];
			j++;
			k++;
		}
	}
	
	while(i<=mid)
	{
		c[k++]=a[i++];
	}
	
	while(j<=end)
	{
		c[k++]=a[j++];
	}
	//copy c back into a
	for(i=begin;i<=end;i++)
	{
		a[i]=c[i-begin];
	}
}



void mergesort(int a[],int begin,int end)
{
	if(begin>=end)
		return;
	else if(end-begin==1) // 2 elements
	{
		if(a[begin]>a[end])
		{
			int temp = a[begin];
			a[begin] = a[end];
			a[end] = temp;
		}
	}
	else
	{
		int mid = (begin+end)/2;
		mergesort(a,begin,mid);
		mergesort(a,mid+1,end);
		merge(a,begin,end);
	}
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("Usage: xsort <filename>\n ");
		exit(-1);
	}
	else
	{
		FILE *fp = fopen(argv[1],"r");
		if(fp == NULL)
		{
			printf("The file was not found\n");
			exit(-1);
		}
		else
		{
			int a[MAX_SIZE];
			int i=1;
			fscanf(fp,"%d",&a[0]);
			while(!feof(fp))
			{
				fscanf(fp,"%d",&a[i]);
				i++;
			}
			fclose(fp);
			int size = i-1;
			mergesort(a,0,size-1);
			//print the sorted numbers
			printf("The sorted numbers are : \n");
			for(i=0;i<size;i++)
			{
				printf("%d ",a[i]);
			}
			printf("\n");
			exit(1); //success
		}
	}
	
}

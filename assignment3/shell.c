#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_QUERY_SIZE 1024
#define MAX_PARAMS 100

int main()
{
    char present_dir[1024]; //assume the length wouldn't exceed 1024
    while(1)
    {
         if (getcwd(present_dir, sizeof(present_dir)) == NULL)
         {
            perror("getcwd() error");
            exit(-1);
         }
        printf("%s> ",present_dir);
        char query[MAX_QUERY_SIZE];
        //scanf("%s",query);
        fgets (query, MAX_QUERY_SIZE-1, stdin);
        char *pch = strtok(query," ");
        char *newline = strchr(pch,'\n');
        if(newline)
            *newline = 0;
        char *command = pch;
        char **args = (char **)malloc(MAX_PARAMS*(sizeof(char *)));
        int no_args=0;
        while(pch!=NULL)
        {
            printf("%s\n",pch);
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
            args[no_args++] = pch;
        }
        int i;
        for(i=0;i<no_args;i++)
            printf("Argument: %s %d\n",args[i],strlen(args[i]));
    }

    
}



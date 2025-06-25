
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include "processQueue.h"





int job_count = 0;
int ncpu,TSLICE;
int running_jobs = 0;

void round_robin_schedule(int signo){
    if(job_count > 0){
        struct Job scheduled_job = dequeue(&readyQueue);
        job_count--;
        kill(scheduled_job.pid, SIGCONT);
        scheduled_job.startTime = time(NULL);
        sleep(TSLICE);
        scheduled_job.waitTimeMillisecs += TSLICE;
        kill(scheduled_job.pid, SIGSTOP);
        scheduled_job.EndTime = time(NULL);
        printf("Job %s, PID %d, waiting time %i\n", scheduled_job.name, scheduled_job.pid,scheduled_job.waitTimeMillisecs);
        printf("start time is %ld secs and end time is %ld secs\n", scheduled_job.startTime,scheduled_job.EndTime);
        printf("\n");

    }
}




void exec_command(char *command)
{   
    int background = 0;
    if(command[strlen(command)-1] == '&'){
        background = 1;
        command[strlen(command)-1] = '\0';
    }
    struct Job new_job;
    strncpy(new_job.name, command, sizeof(new_job.name));
    new_job.waitTimeMillisecs = 0;
    new_job.startTime = time(NULL);
    
    pid_t pid=fork();  //fork called. Child and Parent process created
    int status;
    char *args[100];
    int i = 0;

    char *token = strtok(command, " ");  
    while (token != NULL)
    {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    if (pid < 0)
    {
        printf("fork failed\n"); //exits if fork failed
        exit(1);
    }
    else if (pid == 0)
    {   
        if(background){       
            new_job.pid = fork();
            if(new_job.pid == 0){
                kill(new_job.pid,SIGSTOP);
                enqueue(&readyQueue, new_job);
                exit(1);
            }else if(new_job.pid<0){
                perror("forking gchild error\n");
            }else{
                do {
                    waitpid(new_job.pid, &status, WUNTRACED);   //waits for the child to complete execution and terminate
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
        }else{
            execvp(args[0], args);
        }
    }
    else{
        //parent process called 
        if(!background){
            do {
            waitpid(pid, &status, WUNTRACED);   //waits for the child to complete execution and terminate
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        
    }

}

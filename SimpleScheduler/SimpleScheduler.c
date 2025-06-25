#include "SimpleShellHelper.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
void scheduler();


char* commands[100];


int main(int argc, char* argv[]){
    if(argc != 3){
        perror("NOT ENOUGH ARGUMENTS\n");
    }
    InitialiseQueue(&readyQueue);
    ncpu = atoi(argv[1]);
    TSLICE = atoi(argv[2]);
    char input[MAX_INPUT_SIZE];
    int status;
    while(1){
        printf("Group_70_SimpleScheduler$ ");
        fgets(input,MAX_INPUT_SIZE,stdin);
        input[strlen(input)-1] = '\0';
        if(strcmp(input,"execute") == 0){
            break;
        }
        if(strcmp(input, "") != 0 || strcmp(input, "\0")){
            strncpy(commands[job_count], input, sizeof(input));
            job_count++;
        }
    }
    pid_t dpid = fork();
    if(dpid < 0){
        perror("fork fail\n");
    }
    else if(dpid == 0){
        umask(0);
        pid_t dchildpid = fork();
        if(dchildpid < 0){
            perror("daemon grandchild failure\n");
        }
        if(dchildpid == 0){
            scheduler();
        }
        else{
            waitpid(dchildpid, &status, WUNTRACED); 
        }
    }
    else{
        wait(NULL);
    }

    // while(1){
    //     printf("Group_70_SimpleScheduler$ ");
    //     fgets(input,MAX_INPUT_SIZE,stdin);
    //     input[strlen(input)-1] = '\0';
    //     job_count++;
    //     exec_command(input);
    // }
}

void scheduler(){
    InitialiseQueue(&readyQueue);
    printf("job count is %i\n", job_count);
    for(int k = 0; k < job_count; k++){
        printf("%s\n", commands[k]);
    }
    //starting execution
    int i = 0;
    while(i  < job_count){
        exec_command(commands[i]);
        i++;
    }
    for(int j = 0; j < job_count; j++){
        struct Job temp = dequeue(&readyQueue);
        printf("job is %s, pid is %d\n", temp.name, temp.pid );
    }

}

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>

#include "processQueue.h"

#define MAX_INPUT_SIZE 1024
#define NUM_PROCESSES 100

int NCPU;
int TIME_QUANTUM_US;

struct ProcessInfo {
    pid_t pid;
    struct timespec start_time;
    struct timespec end_time;
    struct timespec wait_time;
};

const char* shm_name = "sharedQueue";


int current_process = 0;
pid_t child_pids[NUM_PROCESSES];
struct ProcessInfo process_info[NUM_PROCESSES];
int c=1.5;




void child_process(char* cmd, int process_no) {
    //record the start time of the process

    // Set the scheduling policy to SCHED_RR
    struct sched_param param;
    param.sched_priority = 1; // Adjust the priority as needed
    if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
        perror("sched_setscheduler");
        exit(1);
    }

    clock_gettime(CLOCK_REALTIME, &process_info[process_no].start_time);
    // Execute the shell command
    if (execl("/bin/sh", "sh", "-c", cmd, NULL) == -1) {
        perror("execl");
        exit(1);
    }
}

void timer_handler(int signo) {
    
    // Handle timer expiration and switch to the next process
    if (kill(child_pids[current_process], SIGSTOP) == -1) {
        perror("kill");
        exit(1);
    }
    printf("executed SIGSTOP for %d TSLICE micro seconds", TIME_QUANTUM_US);

    // Move to the next process in the round-robin order
    current_process = (current_process + 1) % NUM_PROCESSES;

    // Send SIGCONT to the next process to resume it
    if (kill(child_pids[current_process], SIGCONT) == -1) {
        perror("kill");
        exit(1);
    }
}

void makeSubmit(char *str, size_t n) {
    size_t len = strlen(str);

    if (n >= len) {
        // If n is greater than or equal to the length of the string,
        // you effectively have an empty string.
        str[0] = '\0';
    } else {
        // Shift the characters in the string to remove the first n characters
        memmove(str, str + n, len - n + 1); // +1 to include the null-terminator
    }
}


void view_termination(){
    
    FILE *file = fopen("termination.txt", "r");
    printf("%s","");
    if (file == NULL) {
        perror("Error opening the file");
        exit(1);
    }

    char line[MAX_INPUT_SIZE]; // Adjust the buffer size as needed
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
        printf("\n");
    }

    // Close the file
    fclose(file);
    
}


void my_handler(int signum){   //handler for CTRL C
    if (signum== SIGINT){
        printf("Terminating. Ctrl C called.\n");
        view_termination();
        exit(EXIT_SUCCESS);

    }
    //closing shared memory
    munmap(rrq,sizeof(rq));
    shm_unlink(shm_name);
}  



int main(int argc, char* argv[]) {

    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = my_handler;
    if (sigaction(SIGINT, &sa, NULL)==-1){
        perror ("sigaction");
        return 1;
    }
    FILE *termination= fopen("termination.txt", "w");
    fprintf(termination, "%s", "PID      Command      Execution Time     Wait Time\n\n");
    fclose(termination);
    char input[MAX_INPUT_SIZE];
    int job_count = 0;
    if(argc != 3){
        perror("not enough arguments in new function");
    }
    

    // Set up timer
    struct sigaction sa1;
    sa1.sa_handler = timer_handler;
    sa1.sa_flags = 0;
    sigaction(SIGALRM, &sa1, NULL);

    struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TIME_QUANTUM_US;
    timer.it_value = timer.it_interval;

    //setting up shared memory
    rq = shm_open(shm_name,O_CREAT | O_RDWR, 0666);
    ftruncate((int)rq,sizeof(readyQueue));
    printf("working\n");
    struct Queue* temp = (struct Queue*)mmap(0, sizeof(readyQueue), PROT_READ | PROT_WRITE, MAP_SHARED, (int)rq, 0);
    


    while (1) {
        printf("Group_70_SimpleScheduler$ ");
        fgets(input, MAX_INPUT_SIZE, stdin);
        
        input[strlen(input)-1] = '\0';
        const char* s = "submit";
        char* found = strstr(input, s);
        if(found){
            makeSubmit(input, 9);
        }
        NCPU = atoi(argv[1]);
        TIME_QUANTUM_US = atoi(argv[2]);
        int process_no=0;
        if (job_count < NUM_PROCESSES) {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                return 1;
            }

            if (pid == 0) {
                printf("%d\n", process_no);
                child_process(input,process_no);
                process_no++;
                return 0;
            } else {

                clock_gettime(CLOCK_REALTIME, &process_info[process_no].end_time);
                // Calculate and print the execution time
                double execution_time = 
                    (double)(process_info[process_no].end_time.tv_sec - process_info[process_no].start_time.tv_sec) +
                    (double)(process_info[process_no].end_time.tv_nsec - process_info[process_no].start_time.tv_nsec) / 1e9;
                double wait_time= (double) (execution_time/ (TIME_QUANTUM_US * c));
               

                FILE *termination=fopen("termination.txt", "a");
                fprintf(termination, "%d    ", (int)pid);
                fprintf(termination , "%s       ", input);
                fprintf(termination,"%f         ",execution_time);
                fprintf(termination, "%f    ", wait_time );
                fprintf(termination , "\n");
                fclose(termination);


                child_pids[job_count] = pid;
                job_count++;
            }
        } else {
            // All available processes are running, wait for one to finish
            pid_t current_pid = child_pids[current_process];
            
            // Set the timer for the current time quantum
            setitimer(ITIMER_REAL, &timer, NULL);
            
            // Wait for the current process to finish or for the timer to expire
            int status;
            waitpid(current_pid, &status, 0);
            
            // Disable the timer
            timer.it_value.tv_sec = 0;
            timer.it_value.tv_usec = 0;
            setitimer(ITIMER_REAL, &timer, NULL);

            // Move to the next process in the round-robin order
            current_process = (current_process + 1) % NUM_PROCESSES;

            // Launch the new job in place of the finished one
            pid_t new_pid = fork();

            if (new_pid == -1) {
                perror("fork");
                return 1;
            }

            if (new_pid == 0) {
                child_process(input,current_process);
                return 0;
            } else {
                child_pids[current_process] = new_pid;
            }
        }
    }
    //wait time = execution time/time quantum

    return 0;
}

void daemonize_scheduler() {

    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        // Exit the parent process
        exit(EXIT_SUCCESS);
    }
    
    // Create a new session
    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }
    
    // Change the working directory to the root directory
    chdir("/");
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    for(int i = 0; i < NCPU; i++){
        struct Job x = dequeue(&readyQueue);
        kill(x.pid, SIGSTOP);
        sleep(10);
        kill(x.pid,SIGCONT);
        enqueue(&readyQueue,x);      
    }
    
}

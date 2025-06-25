// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <stdbool.h>
// #include <time.h>
// #include <signal.h>


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

#define MAX_INPUT_SIZE 1024
volatile sig_atomic_t is_interrupted = 0;

void exec_command(char *command)
{   

    pid_t pid=fork();  //fork called. Child and Parent process created
    int status;
    // stores the pid, entry time, duration in termination.txt file.
    time_t start_time;
    time(&start_time);
    FILE *termination=fopen("termination.txt", "a");
    fprintf(termination, "%d     ", (int)pid);
    fprintf(termination, "%s        ", command);
    fprintf(termination, "%.3f     ",(double)start_time);

    if (pid < 0)
    {
        printf("fork failed\n"); //exits if fork failed
        exit(1);
    }
    else if (pid == 0)
    {   
        //child process called
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
        execvp(args[0], args);  // the command is executed
        perror("there was a failure in execvp");  //exits if exec fails
        exit(1);
    }
    else{
        //parent process called 
        do {
            waitpid(pid, &status, WUNTRACED);   //waits for the child to complete execution and terminate
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        time_t end_time;
        time(&end_time);
        double difference = difftime(end_time, start_time);
        fprintf(termination, "%.3f    \n", (double)difference);
        fclose(termination);  //closes termination.txt file
    }

}


bool isPiped(char* command)   //checks if a command is piped.
{
    if(strchr(command,'|')!= NULL){
        return true;
    }
    else return false;
}


void exec_piped(char* command)   //Executes the piped commands 
{
    int num_pipes = 0;
    char *pipes[10]; // max 10 commands can be piped at a time.

    // Tokenize the command string based on pipes
    char *token = strtok(command, "|");
    while (token != NULL) {
        pipes[num_pipes] = token;
        num_pipes++;
        token = strtok(NULL, "|");
    }

    int prev_pipe_read = -1; // File descriptor for the previous pipe's read end
    int pipefd[2];           // File descriptors for the current pipe

    for (int i = 0; i < num_pipes; i++) {

        FILE *termination=fopen("termination.txt", "a");
        if (pipe(pipefd) == -1) {
            perror("Pipe failure");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        //time objects for printing during termination
        time_t start_time;
        int status;
        time(&start_time);
        fprintf(termination, "%d    ", (int)pid);
        fprintf(termination,"%s         ",pipes[i]);
        fprintf(termination, "%3f    ", (double)start_time);
        
        
        if (pid <0) {
            perror("Fork failure");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process
            // Close read end of the pipe
            close(pipefd[0]);

            // Redirect stdin if it's not the first command
            if (prev_pipe_read != -1) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }

            // Redirect stdout to the current pipe's write end
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            // Execute the command
            char *args[100]; // Adjust the array size as needed
            int j = 0;

            // Tokenize the individual command based on spaces
            
            
            token = strtok(pipes[i], " ");
            while (token != NULL) {
                args[j] = token;
                j++;
                token = strtok(NULL, " ");
            }
            args[j] = NULL;

            

            execvp(args[0], args);
            perror("execvp"); // Handle error if execvp fails
            exit(EXIT_FAILURE);
        } else { // Parent process
            // Close write end of the pipe
            do {
            waitpid(pid, &status, WUNTRACED);
           } while (!WIFEXITED(status) && !WIFSIGNALED(status));


            time_t end_time;
            time(&end_time);
            double difference = difftime(end_time, start_time);
            fprintf(termination, "%.3f    \n", (double)difference);
            fclose(termination);
            close(pipefd[1]);
            
            // Close the previous pipe's read end
            if (prev_pipe_read != -1) {
                close(prev_pipe_read);
            }

            // Set the previous pipe's read end to the current pipe's read end
            prev_pipe_read = pipefd[0];

        }
    }

    // Read and print the output of the last command in the pipeline
    char output_buffer[1024]; // Adjust the buffer size as needed
    ssize_t num_bytes;

    while ((num_bytes = read(prev_pipe_read, output_buffer, sizeof(output_buffer))) > 0) {
        write(STDOUT_FILENO, output_buffer, num_bytes);
    }

    // Wait for the last child process to finish
    wait(NULL);
}

//checks if there is an & in the command
bool isAnded(char* command)
{
    if(strchr(command,'&')!= NULL){
        return true;
    }
    else return false;
}

void exec_anded(char* command)
{
    char* args[100];
    int i = 0;
    //for loop for execution
    char* token = strtok(command, "&");
    while(token != NULL)
    {
        args[i] = token;
        i++;
        
        exec_command(token);
        token = strtok(NULL,"&");
    }

}

void view_history(){
    FILE *file = fopen("history.txt", "r");

    if (file == NULL) {
        perror("Error opening the file");
        exit(1);
    }

    // Read and print each line from the file
    char line[MAX_INPUT_SIZE]; // Adjust the buffer size as needed
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    // Close the file
    fclose(file);
}


void view_termination(){
    
    FILE *file = fopen("termination.txt", "r");
    printf("%s","");
    if (file == NULL) {
        perror("Error opening the file");
        exit(1);
    }

    // Read and print each line from the file
    char line[MAX_INPUT_SIZE]; // Adjust the buffer size as needed
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    // Close the file
    fclose(file);
    
}

//custom handler for ctrl C
void my_handler(int signum){
    if (signum== SIGINT){
        printf("Terminating. Ctrl C called.");
        view_termination();
        exit(EXIT_SUCCESS);

    }
}

int main()
{   
    int pc=0;
    
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = my_handler;
    if (sigaction(SIGINT, &sa, NULL)==-1){
        perror ("sigaction");
        return 1;
    }

    
    FILE *history= fopen("history.txt", "w");
    fprintf(history, "%s","NO.      Command\n \n");
    fclose(history);
    
    FILE *termination= fopen("termination.txt", "w");
    fprintf(termination, "%s", "PID      Command          Start Time      Duration:\n\n");
    fclose(termination);
    char input[MAX_INPUT_SIZE];
    while(1){
       if (is_interrupted) {
            // Ctrl+C was pressed; continue to the next iteration
            continue;
        }

        printf("Group-48SimpleShell$ ");
        fgets(input,MAX_INPUT_SIZE,stdin);
        input[strlen(input)-1] = '\0';
        pc++;
        //opening history file
        FILE *history= fopen("history.txt", "a");
        fprintf(history, "%d.    ", pc);
        fprintf(history, "%s    \n", input);
        fclose(history);
        if(strcmp(input,"history") == 0){
            view_history();
        }
       
        else{
            //checking for exit command
            if(strcmp(input,"exit") == 0){
                printf("Exiting....\n");
                view_termination();
                break;
            }

            if(isPiped(input) == true){
                exec_piped(input);
            }
            else if(isAnded(input) == true){
                exec_anded(input);
            }
            else {
                
                exec_command(input);
            }
            
        }
    }
    
    return 0;
}

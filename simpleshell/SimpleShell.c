#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE 1024

volatile sig_atomic_t is_interrupted = 0;

void my_handler(int signum){
    if (signum == SIGINT) {
        printf("\n[!] Ctrl+C detected. Terminating shell.\n");
        FILE* term = fopen("termination.txt", "r");
        if (term) {
            char line[MAX_INPUT_SIZE];
            while (fgets(line, sizeof(line), term)) {
                printf("%s", line);
            }
            fclose(term);
        }
        exit(EXIT_SUCCESS);
    }
}

// Logs command execution stats
void log_termination(pid_t pid, const char* cmd, time_t start_time, time_t end_time) {
    FILE* term = fopen("termination.txt", "a");
    if (!term) return;

    fprintf(term, "PID: %d | CMD: %s | Start: %.0f | Duration: %.2f sec\n",
        pid, cmd, (double)start_time, difftime(end_time, start_time));
    fclose(term);
}

// Executes single command (no pipe or &)
void exec_command(char* command) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }

    time_t start_time;
    time_t end_time;

    time(&start_time);  // Parent and child both can use this timestamp

    if (pid == 0) {
        // CHILD
        char* args[100];
        int i = 0;
        char* token = strtok(command, " ");
        while (token) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        execvp(args[0], args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // PARENT
        int status;
        waitpid(pid, &status, 0);
        time(&end_time);
        if (end_time < start_time) end_time = start_time;  // safety
        log_termination(pid, command, start_time, end_time);
    }
}


bool is_piped(const char* cmd){
    return strchr(cmd, '|') != NULL;
}

bool is_background(const char* cmd){
    return strchr(cmd, '&') != NULL;
}

void exec_piped(char* command){
    char* cmds[10];
    int n = 0;

    char* token = strtok(command, "|");
    while (token && n < 10) {
        cmds[n++] = token;
        token = strtok(NULL, "|");
    }

    int prev_fd = -1;
    for (int i = 0; i < n; i++) {
        int pipefd[2];
        pipe(pipefd);
        pid_t pid = fork();

        time_t start_time;
        time_t end_time;
        time(&start_time);

        if (pid == 0) {
            // CHILD
            if (prev_fd != -1) {
                dup2(prev_fd, 0);
                close(prev_fd);
            }
            if (i != n - 1) {
                dup2(pipefd[1], 1);
            }
            close(pipefd[0]);
            close(pipefd[1]);

            char* args[100];
            int j = 0;
            char* tok = strtok(cmds[i], " ");
            while (tok) {
                args[j++] = tok;
                tok = strtok(NULL, " ");
            }
            args[j] = NULL;

            execvp(args[0], args);
            perror("execvp failed in pipe");
            exit(EXIT_FAILURE);
        } else {
            // PARENT
            int status;
            waitpid(pid, &status, 0);
            time(&end_time);
            if (end_time < start_time) end_time = start_time;
            log_termination(pid, cmds[i], start_time, end_time);

            close(pipefd[1]);
            if (prev_fd != -1) close(prev_fd);
            prev_fd = pipefd[0];
        }
    }
}


void exec_background(char* input){
    char* cmds[10];
    int count = 0;
    char* token = strtok(input, "&");
    while (token && count < 10) {
        cmds[count++] = token;
        token = strtok(NULL, "&");
    }

    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            exec_command(cmds[i]);
            exit(0);
        }
    }
}

void view_history() {
    FILE* file = fopen("history.txt", "r");
    if (!file) {
        perror("history file");
        return;
    }

    char line[MAX_INPUT_SIZE];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}

void write_to_history(const char* input, int cmd_num) {
    FILE* file = fopen("history.txt", "a");
    if (file) {
        fprintf(file, "%d. %s\n", cmd_num, input);
        fclose(file);
    }
}

void init_files() {
    FILE* hist = fopen("history.txt", "w");
    if (hist) {
        fprintf(hist, "COMMAND HISTORY\n----------------\n");
        fclose(hist);
    }

    FILE* term = fopen("termination.txt", "w");
    if (term) {
        fprintf(term, "PROCESS TERMINATION LOG\n------------------------\n");
        fclose(term);
    }
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = my_handler;
    sigaction(SIGINT, &sa, NULL);

    init_files();

    char input[MAX_INPUT_SIZE];
    int cmd_count = 0;

    while (1) {
        printf("SimpleShell$ ");
        fflush(stdout);
        clearerr(stdin);
        // if (!fgets(input, sizeof(input), stdin)) continue;
        if (!fgets(input, sizeof(input), stdin)) {
            clearerr(stdin);  // Fixes rare double input edge cases
            continue;
        }

        size_t len = strlen(input);
        if (len == 0 || (len == 1 && input[0] == '\n')) continue;
        input[len - 1] = '\0';  // remove newline

        if (strcmp(input, "exit") == 0) {
            printf("Exiting shell...\n");
            my_handler(SIGINT);
        }

        cmd_count++;
        write_to_history(input, cmd_count);

        if (strcmp(input, "history") == 0) {
            view_history();
            continue;
        }

        // support builtin cd
        if (strncmp(input, "cd ", 3) == 0) {
            char* path = input + 3;
            if (chdir(path) != 0) {
                perror("cd failed");
            }
            continue;
        }

        if (is_piped(input)) {
            exec_piped(input);
        } else if (is_background(input)) {
            exec_background(input);
        } else {
            exec_command(input);
        }
        
    }

    return 0;
}

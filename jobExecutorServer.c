#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>


int jobExecutorServer(int fd) {
    
    // read the number of arguments from jobCommander
    int amount;
    read(fd, &amount, sizeof(int));
    amount -= 1;

    // read the total numbers of chars (a bit larger than the actual one)
    int total_len;
    read(fd, &total_len, sizeof(int));

    // read the string from the pipe
    char arr[total_len];
    read(fd, arr, sizeof(arr));

    // tokenize the string
    char** tokenized = (char **)malloc(amount * sizeof(char*));   
    for (int i = 0 ; i < amount ; i++) {
        tokenized[i] = malloc(total_len * sizeof(char));
    } 
    char* tok = strtok(arr, " ");
    int count = 0;
    while (tok != NULL) {
        if (count == amount) {
            break;
        }
        strcpy(tokenized[count], tok);
        tok = strtok(NULL, " ");
        count++;
    }

    // print the tokens of the string (TODO: delete the prints)
    for (int i = 0 ; i < amount ; i++) {
        printf("i = %s\n", tokenized[i]);
    }


    // free the memory of "tokenized"
    for (int i = 0; i < amount; i++) {
        if (tokenized[i] != NULL) {
            free(tokenized[i]);
        }
    }
}

void signal_handler(int sig) {
    printf("Server got the signal from jobCommander!\n");
}

int main() {
    // create jobExecutorServer.txt
    FILE *job_ex_ser_txt = fopen("jobExecutorServer.txt", "w");
    pid_t pid = getpid();
    fprintf(job_ex_ser_txt, "%d", pid);
    fclose(job_ex_ser_txt);


    // wait signal from jobCommander and then read from the pipe
    int fd = open("comm", O_RDONLY);
    signal(SIGUSR1, signal_handler);
    int exit = 0;
    while (1) {
        pause();
        if (!exit) {
            jobExecutorServer(fd);
            // exit = 1;
        }
    }

    close(fd);

    // delete jobExecutorServer.txt if exited
    int exited = 0;
    if (exited) {
        remove("jobExecutorServer.txt");
    }
}
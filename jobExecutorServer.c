#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "queue.h"


int commands(Queue* myqueue, char** tokenized);

int jobExecutorServer(int fd, Queue* myqueue) {
    
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

    // // print the tokens of the string (TODO: delete the prints)
    // for (int i = 0 ; i < amount ; i++) {
    //     printf("i = %s\n", tokenized[i]);
    // }

    // print info
    printf("String was successfully received and Tokenized!\n");

    // find and exec command
    commands(myqueue, tokenized);

    // free the memory of "tokenized"
    for (int i = 0; i < amount; i++) {
        if (tokenized[i] != NULL) {
            free(tokenized[i]);
        }
    }
}

int issueJob(Queue* myqueue, char* job);

int commands(Queue* myqueue, char** tokenized) {
    if (strcmp(tokenized[0], "issueJob" ) == 0) {
        printf("Inside issueJob!\n");
        issueJob(myqueue, tokenized[1]);
    }
}

int issueJob(Queue* myqueue, char* job) {
    int size = myqueue->size;
    int queuePosition = size;
    static int counter = 0;
    counter++;
    char jobID[100];
    sprintf(jobID, "job_%d", counter);

    // create a job Triplet for the queue
    Triplet* mytriplet = init_triplet(jobID, job, queuePosition);
    enqueue(myqueue, mytriplet);
    print_queue_and_stats(myqueue);
}

void signal_handler(int sig) {
    // print info
    printf("Server got the signal from jobCommander!\n");
}

int main() {
    // create jobExecutorServer.txt
    FILE *job_ex_ser_txt = fopen("jobExecutorServer.txt", "w");
    pid_t pid = getpid();
    fprintf(job_ex_ser_txt, "%d", pid);
    fclose(job_ex_ser_txt);

    // create the Queue for the jobs
    printf("Created the Queue for the jobs!\n");
    Queue* myqueue = createQueue();

    // wait signal from jobCommander and then read from the pipe
    int fd = open("comm", O_RDONLY);
    signal(SIGUSR1, signal_handler);
    int exit = 0;
    while (1) {
        pause();
        if (!exit) {
            jobExecutorServer(fd, myqueue);
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
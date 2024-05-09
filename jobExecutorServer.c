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


char* commands(Queue* myqueue, char** tokenized, char* unix_command);

char* jobExecutorServer(int fd, Queue* myqueue) {

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

    char buffer[total_len];
    for (int i = 1 ; i < amount ; i++) {
        if (i == 1) {
            sprintf(buffer, "%s", tokenized[i]);
        }
        else {
            sprintf(buffer, "%s %s", buffer, tokenized[i]);
        }
    }

    // find and exec command
    char* message = commands(myqueue, tokenized, buffer);

    // free the memory of "tokenized"
    for (int i = 0; i < amount; i++) {
        if (tokenized[i] != NULL) {
            free(tokenized[i]);
        }
    }
    
    return message;
}

Triplet* issueJob(Queue* myqueue, char* job);

char* commands(Queue* myqueue, char** tokenized, char* unix_command) {
    if (strcmp(tokenized[0], "issueJob" ) == 0) {

        Triplet* returned_message = issueJob(myqueue, unix_command);
        char* message = format_triplet(returned_message);
        return message;
    }
    // if (strcmp(tokenized[0], "setConcurrency" ) == 0) {

    //     Triplet* returned_message = issueJob(myqueue, unix_command);
    //     char* message = format_triplet(returned_message);
    //     return message;
    // }
}

Triplet* issueJob(Queue* myqueue, char* job) {
    int size = myqueue->size;
    int queuePosition = size;
    static int counter = 0;
    counter++;
    char jobID[100];
    sprintf(jobID, "job_%d", counter);

    // create a job Triplet for the queue
    Triplet* mytriplet = init_triplet(jobID, job, queuePosition);
    enqueue(myqueue, mytriplet);
    // print_queue_and_stats(myqueue);
    return mytriplet;
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
    Queue* myqueue = createQueue();

    // create the fifo for Server writing - Commander reading
    mkfifo("server", 0666);
    

    // wait signal from jobCommander and then read from the fifo pipe for Commander writing - Server reading
    int fd_commander = open("commander", O_RDONLY);
    signal(SIGUSR1, signal_handler);
    int exit = 0;
    int msg_size;
    while (1) {
        pause();
        if (!exit) {
            // read Commanders pid
            int commander_pid;
            read(fd_commander, &commander_pid, sizeof(int));
            char* message = jobExecutorServer(fd_commander, myqueue);

            // open the fifo for Server writing - Commander reading
            int fd_server = open("server", O_WRONLY);

            // write the size of the message (for the commander to see)
            msg_size = sizeof(char)*(strlen(message)+1);
            write(fd_server, &msg_size, sizeof(int));

            // write the message
            write(fd_server, message, msg_size);

            // close server fifo (will reopen at then next command)
            close(fd_server);
            // exit = 1;
        }
    }

    close(fd_commander);
    

    // delete jobExecutorServer.txt if exited
    // TODO: also need to figure out how to free the memory from the Triplets!
    int exited = 0;
    if (exited) {
        remove("jobExecutorServer.txt");
    }
}
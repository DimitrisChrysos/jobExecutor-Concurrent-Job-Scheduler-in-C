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


char* commands(Queue* myqueue, char** tokenized, char* unix_command, int* concurrency, int* active_processes);

char* jobExecutorServer(int fd, Queue* myqueue, int* concurrency, int* active_processes) {

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
    char* message = commands(myqueue, tokenized, buffer, concurrency, active_processes);

    // free the memory of "tokenized"
    for (int i = 0; i < amount; i++) {
        if (tokenized[i] != NULL) {
            free(tokenized[i]);
        }
    }
    
    return message;
}

Triplet* issueJob(Queue* myqueue, char* job, int* concurrency, int* active_processes);

char* commands(Queue* myqueue, char** tokenized, char* unix_command, int* concurrency, int* active_processes) {
    if (strcmp(tokenized[0], "issueJob" ) == 0) {

        Triplet* returned_message = issueJob(myqueue, unix_command, concurrency, active_processes);
        char* message = format_triplet(returned_message);
        return message;
    }
    else if (strcmp(tokenized[0], "setConcurrency" ) == 0) {

        *concurrency = atoi(tokenized[1]);
        return "-1";    // to not send an actual message back to the Commander 
    }
    else if (strcmp(tokenized[0], "stop" ) == 0) {

        
    }
}

void exec_commands_in_queue(Queue* myqueue, int* concurrency, int* active_processes) {
    if (*active_processes < *concurrency) {
        
        // get the first process "job" of the queue to be executed
        Triplet* mytriplet = myqueue->first_node->value;
        int len = strlen(mytriplet->job);
        char job[len];
        strcpy(job, mytriplet->job);

        // find the "amount" of word in the "job" to be executed
        int amount = 0;
        int total_len = strlen(job) + 1;
        for (int i = 0 ; i < total_len ; i++) {
            if (job[i] == ' ') {
                amount++;
            } 
        }
        amount++;
        
        // tokenize the string "job" using "amount + 1" for the tokenized array to end with NULL
        char** tokenized = (char **)malloc((amount+1) * sizeof(char*));   
        for (int i = 0 ; i < amount ; i++) {
            tokenized[i] = malloc(total_len * sizeof(char));
        } 
        char* tok = strtok(job, " ");
        int count = 0;
        while (tok != NULL) {
            if (count == amount) {
                break;
            }
            strcpy(tokenized[count], tok);
            tok = strtok(NULL, " ");
            count++;
        }
        tokenized[amount] = NULL;
        
        // print the tokens of the string (TODO: delete the prints)
        for (int i = 0 ; i < amount+1 ; i++) {
            printf("i = %s*\n", tokenized[i]);
        }
        
        


        pid_t pid = fork();
        if (pid == 0) { // child process
            // char *args[]={job, NULL};
            execvp(tokenized[0], tokenized);
        }
        if (pid > 0) {  // parent process

            // wait for child signal


            // free the memory of "tokenized"
            for (int i = 0; i < amount; i++) {
                if (tokenized[i] != NULL) {
                    free(tokenized[i]);
                }
            }
        }

    }
}

Triplet* issueJob(Queue* myqueue, char* job, int* concurrency, int* active_processes) {
    int size = myqueue->size;
    int queuePosition = size;
    static int counter = 0;
    counter++;
    char jobID[100];
    sprintf(jobID, "job_%d", counter);

    // create a job Triplet for the queue
    Triplet* mytriplet = init_triplet(jobID, job, queuePosition, -1);
    enqueue(myqueue, mytriplet);
    // print_queue_and_stats(myqueue);

    exec_commands_in_queue(myqueue, concurrency, active_processes);

    return mytriplet;
}

void signal_handler(int sig) {
    // print info
    printf("Server got the signal %d from jobCommander!\n", sig);
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
    
    int concurrency = 1;
    int active_processes = 0;

    // wait signal from jobCommander and then read from the fifo pipe for Commander writing - Server reading
    int fd_commander = open("commander", O_RDONLY);
    signal(SIGUSR1, signal_handler);
    // signal(SIGUSR2, signal_handler);
    int exit = 0;
    int msg_size;
    while (1) {
        pause();
        if (!exit) {
            // read Commanders pid
            int commander_pid;
            read(fd_commander, &commander_pid, sizeof(int));
            char* message = jobExecutorServer(fd_commander, myqueue, &concurrency, &active_processes);
            printf("concurrency = %d\n", concurrency);

            // open the fifo for Server writing - Commander reading
            int fd_server = open("server", O_WRONLY);

            // if message == -1, send -1 to notify that there is no actual message
            // else send the message
            if (strcmp(message, "-1") == 0) {
                int send_minus_one = -1;
                write(fd_server, &send_minus_one, sizeof(int));
            }
            else {
                // write the size of the message (for the commander to see)
                msg_size = sizeof(char)*(strlen(message)+1);
                write(fd_server, &msg_size, sizeof(int));

                // write the message
                write(fd_server, message, msg_size);
            }


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
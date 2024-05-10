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

typedef struct server_info ServerInfo;
typedef struct server_info {
    int fd_commander;
    Queue* myqueue;
    int concurrency;
    int active_processes;
} ServerInfo;

ServerInfo *info;

char* commands(char** tokenized, char* unix_command);

void jobExecutorServer() {

    
    // read the number of arguments from jobCommander
    int amount;
    read(info->fd_commander, &amount, sizeof(int));
    amount -= 1;

    // read the total numbers of chars (a bit larger than the actual one)
    int total_len;
    read(info->fd_commander, &total_len, sizeof(int));

    // read the string from the pipe
    char arr[total_len];
    read(info->fd_commander, arr, sizeof(arr));

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

    // create a buffer to save the whole unix_command from the tokens
    char buffer[total_len];
    for (int i = 1 ; i < amount ; i++) {
        if (i == 1) {
            sprintf(buffer, "%s", tokenized[i]);
        }
        else {
            sprintf(buffer, "%s %s", buffer, tokenized[i]);
        }
    }

    
    // find and exec commands
    char* message = commands(tokenized, buffer);
    printf("message: %s\n", message);

    // free the memory of "tokenized"
    for (int i = 0; i < amount; i++) {
        if (tokenized[i] != NULL) {
            free(tokenized[i]);
        }
    }
    
    // return message;

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
        int msg_size = sizeof(char)*(strlen(message)+1);
        write(fd_server, &msg_size, sizeof(int));

        // write the message
        write(fd_server, message, msg_size);
    }

    // close server fifo (will reopen at then next command)
    close(fd_server);
}

Triplet* issueJob(char* job);

char* commands(char** tokenized, char* unix_command) {
    if (strcmp(tokenized[0], "issueJob" ) == 0) {
        Triplet* returned_message = issueJob(unix_command);
        char* message = format_triplet(returned_message);
        return message;
    }
    else if (strcmp(tokenized[0], "setConcurrency" ) == 0) {

        info->concurrency = atoi(tokenized[1]);
        return "-1";    // to not send an actual message back to the Commander 
    }
    else if (strcmp(tokenized[0], "stop" ) == 0) {

        
    }
}

void exec_commands_in_queue() {
    if (info->active_processes < info->concurrency) {
        
        // get the first process "job" of the queue to be executed
        Triplet* mytriplet = info->myqueue->first_node->value;
        int len = strlen(mytriplet->job);
        char job[len];
        strcpy(job, mytriplet->job);

        // find the "amount" of words in the "job" to be executed
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
        
        // create a new process and replace it using execvp 
        // to execute the wanted process
        pid_t pid = fork();
        if (pid == 0) { // child process
            // char *args[]={job, NULL};
            execvp(tokenized[0], tokenized);
        }
        
        // free the memory of "tokenized"
        for (int i = 0; i < amount; i++) {
            if (tokenized[i] != NULL) {
                free(tokenized[i]);
            }
        }
    }
}

Triplet* issueJob(char* job) {

    // format the triplet variables
    int size = info->myqueue->size;
    int queuePosition = size;
    static int counter = 0;
    counter++;
    char jobID[100];
    sprintf(jobID, "job_%d", counter);

    // create a job Triplet for the queue
    Triplet* mytriplet = init_triplet(jobID, job, queuePosition);
    enqueue(info->myqueue, mytriplet);
    // print_queue_and_stats(myqueue);

    // run the commands, if possible
    exec_commands_in_queue();

    return mytriplet;
}


void create_jobExecutorServerTEXT() {
    // create jobExecutorServer.txt
    FILE *job_ex_ser_txt = fopen("jobExecutorServer.txt", "w");
    pid_t pid = getpid();
    fprintf(job_ex_ser_txt, "%d", pid);
    fclose(job_ex_ser_txt);
}

int main() {
    
    create_jobExecutorServerTEXT();

    // create the Queue for the jobs
    Queue* myqueue = createQueue();

    // create the fifo for Server writing - Commander reading
    mkfifo("server", 0666);

    // open the fifo pipe for Commander writing - Server reading  
    int fd_commander = open("commander", O_RDONLY);

    // Init the ServerInfo struct and set the global pointer
    ServerInfo myServerInfo = {fd_commander, myqueue, 1, 0};
    info = &myServerInfo;

    // wait signal from jobCommander and then read from the fifo pipe for Commander writing - Server reading
    signal(SIGUSR1, jobExecutorServer);

    // pause the process after every signal inside a while loop, to keep the server open
    while (1) {
        pause();
    }

    // close the fifo pipe for Commander writing - Server reading 
    close(fd_commander);
    

    // delete jobExecutorServer.txt if exited
    // TODO: also need to figure out how to free the memory from the Triplets!
    int exited = 0;
    if (exited) {
        remove("jobExecutorServer.txt");
    }
}
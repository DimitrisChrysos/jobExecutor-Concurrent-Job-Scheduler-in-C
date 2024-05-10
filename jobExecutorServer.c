#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
// #include "queue.h"
#include "ServerCommands.h"

ServerInfo *info;

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
    
        // free the memory for message
        free(message);
    }


    // close server fifo (will reopen at then next command)
    close(fd_server);
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

    // create the running Queue for the processes (jobs), that are currently running
    Queue* running_queue = createQueue();

    // create the fifo for Server writing - Commander reading
    mkfifo("server", 0666);

    // open the fifo pipe for Commander writing - Server reading  
    int fd_commander = open("commander", O_RDONLY);

    // Init the ServerInfo struct and set the global pointer
    ServerInfo myServerInfo = {fd_commander, myqueue, 1, 0, running_queue};
    info = &myServerInfo;

    // wait signal from jobCommander and then read from the fifo pipe for Commander writing - Server reading
    signal(SIGUSR1, jobExecutorServer);
    // struct sigaction commander;
    // commander.sa_handler = jobExecutorServer;
    // sigemptyset(&commander.sa_mask);
    // commander.sa_flags = 0;
    // sigaction(SIGUSR1, &commander, NULL);

    
    // // wait signal from a child process 
    // signal(SIGCHLD, exec_commands_in_queue);
    struct sigaction child_act;
    child_act.sa_handler = exec_commands_in_queue;
    sigemptyset(&child_act.sa_mask);
    child_act.sa_flags = 0;
    sigaction(SIGCHLD, &child_act, NULL);


    // keep the server open
    while (1) {
        // pause();
        waitpid(0, NULL, WNOHANG);
        // printf("child_pd = %d\n", child_pid);
        // printf("myqueue_size = %d | running_queue_size = %d\n", info->myqueue->size, info->running_queue->size);
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
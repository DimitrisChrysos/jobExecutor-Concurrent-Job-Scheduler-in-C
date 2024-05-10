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
#include "queue.h"

typedef struct server_info ServerInfo;
typedef struct server_info {
    int fd_commander;
    Queue* myqueue;
    int concurrency;
    int active_processes;
    Queue* running_queue;
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

Triplet* issueJob(char* job);

char* stop_job(char** tokenized);

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
        
        char* message = stop_job(tokenized);
        return message; 
    }
}

char* stop_job(char** tokenized) {
    char* jobID = tokenized[1];
    
    // Check if the process with jobID is currently running
    int running = 0;
    int qSize = info->running_queue->size;
    Node* temp_node = info->running_queue->first_node;
    for (int i = 0 ; i < qSize ; i++) {
        Triplet* tempTriplet = temp_node->value;
        if (strcmp(tempTriplet->jobID, jobID) == 0) {
            running = 1;
            break;
        }
        temp_node = temp_node->child;
    }
    
    // print_queue_and_stats(info->running_queue);

    // if found currently running, terminate the process and remove it from the
    // running queue (also deallocate the memory for the node)
    // else remove it from the main queue
    Triplet* tempTriplet;
    char* buffer = (char*)malloc(sizeof(char)*(strlen(jobID)+30));
    if (running) {
        if (temp_node->parent != NULL) {
            temp_node->parent->child = temp_node->child;
            tempTriplet = temp_node->value;
            kill(tempTriplet->pid, SIGTERM);
            delete_triplet(tempTriplet);
            free(temp_node);
        }
        else if (temp_node->child == NULL) {
            tempTriplet = dequeue(info->running_queue);
            kill(tempTriplet->pid, SIGTERM);
            delete_triplet(tempTriplet);
        }
        else {
            info->running_queue->first_node = temp_node;
            temp_node->child->parent = NULL;
            tempTriplet = temp_node->value;
            kill(tempTriplet->pid, SIGTERM);
            delete_triplet(tempTriplet);
            free(temp_node);
        }
        sprintf(buffer, "%s terminated", jobID);
        return buffer;
    }
    else {
        // Check if the process with jobID is waiting
        int waiting = 0;
        int qSize = info->myqueue->size;
        Node* temp_node = info->myqueue->first_node;
        for (int i = 0 ; i < qSize ; i++) {
            Triplet* tempTriplet = temp_node->value;
            if (strcmp(tempTriplet->jobID, jobID) == 0) {
                waiting = 1;
                break;
            }
            temp_node = temp_node->child;
        }

        // if it's waiting, remove it from myqueue
        if (waiting) {
            if (temp_node->parent != NULL) {
                temp_node->parent->child = temp_node->child;
                tempTriplet = temp_node->value;
                kill(tempTriplet->pid, SIGTERM);
                delete_triplet(tempTriplet);
                free(temp_node);
            }
            else if (temp_node->child == NULL) {
                tempTriplet = dequeue(info->myqueue);
                kill(tempTriplet->pid, SIGTERM);
                delete_triplet(tempTriplet);
            }
            else {
                info->myqueue->first_node = temp_node;
                temp_node->child->parent = NULL;
                tempTriplet = temp_node->value;
                kill(tempTriplet->pid, SIGTERM);
                delete_triplet(tempTriplet);
                free(temp_node);
            }
            sprintf(buffer, "%s removed", jobID);
            return buffer;
        }
    }
    sprintf(buffer, "%s not found", jobID);
    return buffer;
}

void exec_commands_in_queue(int sig) {

    // this means, that the function was called because an old process ended
    // so we need to remove the first item from the running queue
    if (sig == SIGCHLD) {
        Triplet* removed = dequeue(info->running_queue);
        delete_triplet(removed);
    }

    // printf("info->myqueue->size = %d\n", info->myqueue->size);

    if (info->active_processes < info->concurrency && info->myqueue->size != 0) {
        
        // execute processes until capacity (info->concurrency) reached
        for (int i = info->active_processes ; i < info->concurrency ; i++) {

            // Check if there are no processes left
            if (info->myqueue->size == 0) {
                break;
            }
            // sleep(1);

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
            char** tokenized = (char **)malloc((amount+1)*sizeof(char*));
            for (int i = 0 ; i < amount ; i++) {
                tokenized[i] = (char*)malloc((total_len+1) * sizeof(char));
            }
            tokenized[amount] = NULL;
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
            

            // remove the front process from the queue and add it to the running_queue
            Triplet* removed_triplet = dequeue(info->myqueue);
            enqueue(info->running_queue, removed_triplet);

            // // for test purposes (to delete later)
            // char* temp_triplet = format_triplet(removed_triplet);
            // printf("triplet string: %s\n", temp_triplet);
            // free(temp_triplet);

            // and replace the queuePosition for every item of the main queue
            int qSize = info->myqueue->size;
            Node* temp_node = info->myqueue->first_node;
            for (int i = 0 ; i < qSize ; i++) {
                Triplet* tempTriplet = temp_node->value;
                tempTriplet->queuePosition = i;
                temp_node = temp_node->child;
            }
            
            // create a new process and replace it using execvp 
            // to execute the wanted process
            pid_t pid = fork();
            if (pid == 0) { // child process
                // printf("child pid = %d\n", getpid());

                // execute the wanted process
                execvp(tokenized[0], tokenized);
            }

            // printf("parent pid: %d | child pid: %d\n", getpid(), pid);
            
            // add the pid of the process to the triplet (usefull to terminate the process)
            removed_triplet->pid = pid;
            
            // free the memory of "tokenized"
            for (int i = 0; i < amount + 1; i++) {
                if (tokenized[i] != NULL) {
                    free(tokenized[i]);
                }
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
    Triplet* mytriplet = init_triplet(jobID, job, queuePosition, -1);
    enqueue(info->myqueue, mytriplet);
    
    // print_queue_and_stats(myqueue);

    // run the commands, if possible
    exec_commands_in_queue(-1);
    

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
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
#include "ServerCommands.h"
#include <semaphore.h>

// pointer allowing global access to the information of the Server 
ServerInfo *info;

// signal handler called when the Commander sends signal SIGUSR1 to execute commands
void jobExecutorServer() {

    // read the pid of the commander
    int com_pid;
    read(info->fd_commander, &com_pid, sizeof(int));

    // read the number of arguments from jobCommander
    int amount;
    read(info->fd_commander, &amount, sizeof(int));
    amount -= 1;
    
    // read if the arguments will be send in multiple packets or not
    // packets = 1 -> not || packets > 1 -> yes
    int packets;
    read(info->fd_commander, &packets, sizeof(int));
    
    int total_len;
    char* arr;
    int first_packet = 1;
    if (packets > 1) {
        
        // create a semaphore for the Server wait
        info->serverSem = sem_open("/serverSem", O_CREAT, 0666, 0);
        if (info->serverSem == SEM_FAILED) {
            printf("server: serverSem failed\n");
        }

        // create a semaphore for the Commander wait
        info->commanderSem = sem_open("/commanderSem", O_CREAT, 0666, 0);
        if (info->commanderSem == SEM_FAILED) {
            printf("server: commanderSem failed\n");
        }

        // send signal to Commander to notify Server has created the semaphore
        // and it is ready to read
        kill(com_pid, SIGUSR2);
        
        // init total_len
        total_len = 0;
        
        // read the len of each packet and the packet itself
        for (int i = 0 ; i < packets ; i++) {

            // wait for commander to post the semaphore (send the package)
            sem_wait(info->serverSem);

            // read the packet_len
            int packet_len;
            read(info->fd_commander, &packet_len, sizeof(int));
            total_len += packet_len;

            // read the packet
            if (first_packet) {
                arr = (char*)malloc(sizeof(char)*packet_len);
                read(info->fd_commander, arr, sizeof(char)*packet_len);
                first_packet = 0;
            }
            else {
                char temp_arr[packet_len];
                read(info->fd_commander, temp_arr, sizeof(char)*packet_len);
                arr = (char*)realloc(arr, sizeof(char)*total_len);
                strcat(arr, temp_arr);
            }

            sem_post(info->commanderSem);

        }

        // close the semaphore
        sem_close(info->serverSem);
        
    }
    else if (packets == 1) {

        // read the total numbers of chars (a bit larger than the actual one)
        read(info->fd_commander, &total_len, sizeof(int));

        // read the string from the pipe
        arr = (char*)malloc(total_len);
        read(info->fd_commander, arr, total_len);
    }

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

    int real_amount;
    for (int i = 0 ; i < amount ; i++) {
        if (strcmp(tokenized[i],"") == 0) {
            real_amount = i + 1;
            break;
        }
    }

    // find and exec commands
    char* message = commands(tokenized, buffer);
    free(arr);

    // free the memory of "tokenized"
    for (int i = 0; i < amount; i++) {
        if (tokenized[i] != NULL) {
            free(tokenized[i]);
        }
    }

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

    // close server fifo (will re-open at then next command)
    close(fd_server);
}


void remove_pid_from_run_queue(pid_t mypid);


// create jobExecutorServer.txt
void create_jobExecutorServerTEXT() {
    
    FILE *job_ex_ser_txt = fopen("jobExecutorServer.txt", "w");
    pid_t pid = getpid();
    fprintf(job_ex_ser_txt, "%d", pid);
    fclose(job_ex_ser_txt);
}

int main() {
    
    // create the txt file
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
    ServerInfo myServerInfo = {fd_commander, myqueue, 1, running_queue, 1, -1, NULL};
    info = &myServerInfo;

    // wait signal from jobCommander and then read from the fifo pipe for Commander writing - Server reading
    signal(SIGUSR1, jobExecutorServer);
    
    // wait signal from a child process
    struct sigaction child_act;
    child_act.sa_handler = exec_commands_in_queue;
    sigemptyset(&child_act.sa_mask);
    child_act.sa_flags = 0;
    sigaction(SIGCHLD, &child_act, NULL);

    // keep the server open (waiting for the signals)
    while (info->open) {
        int child_pid = waitpid(0, NULL, WNOHANG);
        
        if (child_pid != 0 && child_pid != -1) {
            if (info->killed_pid == -1) {

                // if child process finished (without stop jobID), remove it from running queue
                remove_pid_from_run_queue(child_pid);
            }
            else {
                info->killed_pid = -1;
            }
        }
    }

    // close the fifo pipe for Commander writing - Server reading 
    close(fd_commander);
    
    // remove jobExecutorServer.txt
    remove("jobExecutorServer.txt");

    // free the memory from the Triplets - Queues!
    if (info->myqueue->size > 0) {  // free the memory from the main queue
        int qSize = info->myqueue->size;
        Node* temp_node = info->myqueue->first_node;
        for (int i = 0 ; i < qSize ; i++) {
            Triplet* tempTriplet = temp_node->value;
            delete_triplet(tempTriplet);
            temp_node = temp_node->child;
        }
    }
    deleteQueue(info->myqueue);

    if (info->running_queue->size > 0) {    // free the memory from the running queue
        int qSize = info->running_queue->size;
        Node* temp_node = info->running_queue->first_node;
        for (int i = 0 ; i < qSize ; i++) {
            Triplet* tempTriplet = temp_node->value;
            delete_triplet(tempTriplet);
            temp_node = temp_node->child;
        }
    }
    deleteQueue(info->running_queue);
}

void remove_pid_from_run_queue(pid_t mypid) {

    // Find the node to be removed
    int qSize = info->running_queue->size;
    Node* temp_node = info->running_queue->first_node;
    for (int i = 0 ; i < qSize ; i++) {
        Triplet* tempTriplet = temp_node->value;
        if (tempTriplet->pid == mypid) {
            break;
        }
        temp_node = temp_node->child;
    }
    
    // delete the Triplet and free the node 
    Triplet *tempTriplet;
    if (temp_node->parent != NULL) {
        if (temp_node->child == NULL) {
            info->running_queue->last_node = temp_node->parent;
        }
        else {
            temp_node->child->parent = temp_node->parent;
        }
        temp_node->parent->child = temp_node->child;
        tempTriplet = temp_node->value;
        info->running_queue--;
        delete_triplet(tempTriplet);
        free(temp_node);
    }
    else if (temp_node->child == NULL) {
        tempTriplet = dequeue(info->running_queue);
        delete_triplet(tempTriplet);
    }
    else {
        info->running_queue->first_node = temp_node->child;
        temp_node->child->parent = NULL;
        tempTriplet = temp_node->value;
        info->running_queue--;
        delete_triplet(tempTriplet);
        free(temp_node);
    }
}
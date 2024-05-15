#include "queue.h"
#include <semaphore.h>

// struct to save info for the server (global access through pointer)
typedef struct server_info ServerInfo;
typedef struct server_info {
    int fd_commander;   // fd for the Commander writting fifo pipe
    Queue* myqueue; // waiting queue
    int concurrency;    // concurrency number
    Queue* running_queue;   // running queue
    int open;   // if the server is open
    int killed_pid; // the child process pid, killed by the stop command
    sem_t* serverSem;   // the semaphore for the Server waiting
    sem_t* commanderSem;    // the semaphore for the Commander waiting
} ServerInfo;

// struct to save the information of the server
extern ServerInfo *info;

// matches the command given to it's corresponding function or functionality
char* commands(char** tokenized, char* unix_command);

// the signal_handler fot the SIGCHILD signal - executes the processes in queue
void exec_commands_in_queue(int sig);

// function for the issueJob command
Triplet* issueJob(char* job);

// function for the stop jobID command
char* stop_job(char** tokenized);

// function for the poll command
char* poll(char** tokenized);
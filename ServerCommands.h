#include "queue.h"
#include <semaphore.h>

// struct to save info for the server (global access through pointer)
typedef struct server_info ServerInfo;
typedef struct server_info {
    int fd_commander;
    Queue* myqueue;
    int concurrency;
    Queue* running_queue;
    int open;
    int killed_pid;
    sem_t* serverSem;
    sem_t* commanderSem;
} ServerInfo;

extern ServerInfo *info;

// matches the command given to it's corresponding function
char* commands(char** tokenized, char* unix_command);

// the signal_handler after SIGCHILD signal - executes the processes in queue
void exec_commands_in_queue(int sig);

// function for the issueJob command
Triplet* issueJob(char* job);

// function for the stop jobID command
char* stop_job(char** tokenized);

// function for the poll command
char* poll(char** tokenized);
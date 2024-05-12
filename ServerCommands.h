#include "queue.h"

typedef struct server_info ServerInfo;
typedef struct server_info {
    int fd_commander;
    Queue* myqueue;
    int concurrency;
    Queue* running_queue;
    int open;
    int killed_pid;
} ServerInfo;

extern ServerInfo *info;

char* commands(char** tokenized, char* unix_command);

void exec_commands_in_queue(int sig);

Triplet* issueJob(char* job);

char* stop_job(char** tokenized);

char* poll(char** tokenized);
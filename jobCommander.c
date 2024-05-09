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


int jobCommander(int argc, char *argv[]) {

    // check if the server is active
    FILE* myfile = fopen("jobExecutorServer.txt", "r");
    pid_t p;
    if (myfile == NULL) {
        // file does not exist -> server is not active, activate the server
        p = fork();
        if (p == 0) {   // child process
            char *args[]={"./jobExecutorServer", NULL};
            execvp(args[0], args);
        }

        // create the fifo
        mkfifo("comm", 0666);

        // print info
        printf("Server was not active...\nServer activated and fifo pipe created!\n");
    }
    else {
        // file exists -> server is active, fifo already exists
        fscanf(myfile, "%d", &p);
        fclose(myfile);

        // print info
        printf("Server is active!\n");
    }


    // open the fifo
    int fd = open("comm", O_WRONLY);
    
    // write the number of strings for the pipe
    write(fd, &argc, sizeof(int));

    // write the total number of chars that will be in the pipe
    int total_len = 0;
    for (int i = 1 ; i < argc ; i++) {
        total_len += strlen(argv[i]);
    }
    total_len += argc - 1;     // add some extra chars for the " " between words
    total_len += total_len*0.1;     // add 10% extra chars for safety
    write(fd, &total_len, sizeof(int));

    // write the strings in the pipe
    for (int i = 1 ; i < argc ; i++) {
        write(fd, argv[i], strlen(argv[i]));
        write(fd, " ", 1);
        // printf("Wrote: %s\n", argv[i]);
    }

    // give the signal to jocExecutorServer
    kill(p, SIGUSR1);

    // close the fifo
    close(fd);
}







void main(int argc, char *argv[]) {
    
    
    jobCommander(argc, argv);
}
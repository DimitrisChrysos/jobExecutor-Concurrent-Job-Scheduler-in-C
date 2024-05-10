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

void signal_handler(int sig) {
    // print info
    printf("Commander got the signal from Server!\n");
}

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

        // create the fifo for Commander writing - Server reading
        mkfifo("commander", 0666);

        // print info
        printf("Server was not active... Server activated!\n");
    }
    else {
        // file exists -> server is active, fifo already exists
        fscanf(myfile, "%d", &p);
        fclose(myfile);

        // print info
        printf("Server is active!\n");
    }


    // open the fifo for Commander writing - Server reading
    int fd_commander = open("commander", O_WRONLY);

    // write the number of strings for the pipe
    write(fd_commander, &argc, sizeof(int));

    // write the total number of chars that will be in the pipe
    int total_len = 0;
    for (int i = 1 ; i < argc ; i++) {
        total_len += strlen(argv[i]);
    }
    total_len += argc - 1;     // add some extra chars for the " " between words
    total_len += total_len*0.1;     // add 10% extra chars for safety
    write(fd_commander, &total_len, sizeof(int));

    // write the strings in the pipe
    for (int i = 1 ; i < argc ; i++) {
        write(fd_commander, argv[i], strlen(argv[i]));
        write(fd_commander, " ", 1);
        // printf("Wrote: %s\n", argv[i]);
    }


    // give the signal to jocExecutorServer
    kill(p, SIGUSR1);

    // open the fifo for Server writing - Commander reading
    int fd_server = open("server", O_RDONLY);

    // read from Server the returned message
    // if msg_size == -1, there is no actual message send
    int msg_size;
    read(fd_server, &msg_size, sizeof(int)); // read the size of the message
    if (msg_size != -1) {
        char* message = (char*)malloc(sizeof(char)*msg_size);
        read(fd_server, message, msg_size); // read the message
        printf("Message received from the Server to the Commander: %s\n", message);
    }
    

    // close the fifos
    close(fd_commander);
    close(fd_server);
}




void main(int argc, char *argv[]) {
    
    
    jobCommander(argc, argv);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
// #include <time.h>  


int jobCommander(int argc, char *argv[]) {

    // check is the server is active
    FILE* myfile = fopen("jobExecutorServer.txt", "r");
    pid_t p;
    if (myfile == NULL) {
        // file does not exist -> server is not active, activate the server
        p = fork();
        if (p == 0) {   // child process
            char *args[]={"./jobExecutorServer", NULL};
            execvp(args[0], args);
        }
    }
    else {
        // file exists -> server is active
        fscanf(myfile, "%d", &p);
        fclose(myfile);
    }


    // create the fifo
    mkfifo("comm", 0666);
    int fd = open("comm", O_WRONLY);
    

    // write the number of strings in the pipe
    write(fd, &argc, sizeof(int));

    // write the strings in the pipe
    for (int i = 1 ; i < argc ; i++) {
        write(fd, argv[i], strlen(argv[i]));
        write(fd, " ", 1);
        // printf("Wrote: %s\n", argv[i]);
    }

    kill(p, SIGUSR1);
    close(fd);
}







void main(int argc, char *argv[]) {
    
    
    jobCommander(argc, argv);
}
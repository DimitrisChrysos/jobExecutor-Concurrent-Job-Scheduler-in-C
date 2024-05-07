#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


int jobCommander(int argc, char *argv[]) {
    FILE* myfile = fopen("jobExecutorServer.txt", "r");
    if (myfile == NULL) {
        // file does not exist -> server is not active, activate the server
        pid_t p = fork();
        if (p == 0) {   // child process
            char *args[]={"./jobExecutorServer", NULL};
            execvp(args[0], args);
        }



    }
    else {
        // file exists -> server is active
        fclose(myfile);
        
    }




    // for (int i=0; i < argc ; i++) {
    //     printf("%s\n", argv[i]);
    // }
}







void main(int argc, char *argv[]) {
    
    
    jobCommander(argc, argv);
}
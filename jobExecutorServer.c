#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int jobExecutorServer() {

}



int main() {
    // create jobExecutorServer.txt
    FILE *job_ex_ser_txt = fopen("jobExecutorServer.txt", "w");
    pid_t pid = getpid();
    fprintf(job_ex_ser_txt, "%d", pid);
    fclose(job_ex_ser_txt);


    // do staff needed
    jobExecutorServer();


    // delete jobExecutorServer.txt if
    int ended = 0;
    if (ended) {
        remove("jobExecutorServer.txt");
    }
}
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

#define PACKET_CAPACITY 100 // the max capacity of each packet

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
    }
    else {
        // file exists -> server is active, fifo already exists
        fscanf(myfile, "%d", &p);
        fclose(myfile);
    }


    // open the fifo for Commander writing - Server reading
    int fd_commander = open("commander", O_WRONLY);

    // if packets > 1 -> server has way to many arguments and needs to send arguments in multiple packets
    // else if packets = 1 -> server doesn't need to send arguments in multiple packets
    int packets;  // total packets needed to send
    if (argc > PACKET_CAPACITY) {
        packets = argc / PACKET_CAPACITY;
        if ((argc % PACKET_CAPACITY) > 0) {
            packets++;
        }

        // write the amount of packets needed to send
        write(fd_commander, &packets, sizeof(int));
        
        // write each packet
        int index = 0;
        for (int i = 0 ; i < packets ; i++) {
            int packet_len = 0;
            for (int j = index ; j < index + PACKET_CAPACITY ; j++) {
                packet_len += strlen(argv[j]);
                packet_len += packet_len;     // add some extra chars for the " " between words
                packet_len += packet_len*0.1;     // add 10% extra chars for safety
            }

            // write the len of the packet
            write(fd_commander, &packet_len, sizeof(int));

            // write the packet in the pipe
            for (int j = index ; j < index + PACKET_CAPACITY ; j++) {
                write(fd_commander, argv[j], strlen(argv[j]));
                write(fd_commander, " ", 1);
            }
            
            // add packet_capacity to the current argument index
            index += PACKET_CAPACITY;
            printf("packet_index = %d\n", i);
        }
    }
    else {
        // write the amount of packets needed to send is one, so no multiple packets
        packets = 1;
        write(fd_commander, &packets, sizeof(int));

        // write the number of strings for the pipe
        write(fd_commander, &argc, sizeof(int));

        // write the total number of chars that will be in the pipe
        int total_len = 0;
        for (int i = 1 ; i < argc ; i++) {
            total_len += strlen(argv[i]);
        }
        total_len += total_len;     // add some extra chars for the " " between words
        total_len += total_len*0.1;     // add 10% extra chars for safety
        write(fd_commander, &total_len, sizeof(int));

        // write the strings in the pipe
        for (int i = 1 ; i < argc ; i++) {
            write(fd_commander, argv[i], strlen(argv[i]));
            write(fd_commander, " ", 1);
        }
    }
    
    // give the signal to jobExecutorServer
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
        printf("%s\n", message); // print the message
    }
    

    // close the fifos
    close(fd_commander);
    close(fd_server);
}




void main(int argc, char *argv[]) {
    
    jobCommander(argc, argv);
}
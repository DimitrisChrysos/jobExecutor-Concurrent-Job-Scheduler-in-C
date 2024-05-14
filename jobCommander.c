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
#include <semaphore.h>

#define PACKET_CAPACITY 100 // the max capacity of each packet

void useless_sig_handler() {}

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
    
    // write the pid of the commander
    int mypid = getpid();
    write(fd_commander, &mypid, sizeof(int));

    // write the number of strings for the pipe
    write(fd_commander, &argc, sizeof(int));
    
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
        
        // send signal to make the Server start executing 
        kill(p, SIGUSR1);

        // wait for the Server to send signal, notifying that it has created the semaphore
        int sig;
        sigset_t usr2;
        sigemptyset(&usr2);
        sigaddset(&usr2, SIGUSR2);
        signal(SIGUSR2, NULL);
        sigprocmask(SIG_BLOCK, &usr2, NULL);
        sigwait(&usr2, &sig);

        // unblock SIGUSR2, signal
        sigprocmask(SIG_UNBLOCK, &usr2, NULL);

        // open the semaphore for the Server wait 
        sem_t *serverSem = sem_open("/serverSem", 0);
        if (serverSem == SEM_FAILED) {
            printf("commander: serverSem failed\n");
        }

        // open the semaphore for the Commander wait 
        sem_t *commanderSem = sem_open("/commanderSem", 0);
        if (commanderSem == SEM_FAILED) {
            printf("commander: commanderSem failed\n");
        }
        
        // printf("total packets = %d\n", packets);

        // write each packet
        int index = 0;
        for (int i = 0 ; i < packets ; i++) {

            printf("COMMANDER: trying to sent packet (with index: %d of %d) \n", i+1, packets);
            
            int broke = 0 ;
            int packet_len = 0;
            for (int j = index ; j < index + PACKET_CAPACITY ; j++) {
                if (argv[j] == NULL) {
                    printf("last argument = %s\n", argv[j-1]);
                    broke = j;
                    break;
                }
                
                packet_len += strlen(argv[j]) + 1;

                if (j == index + PACKET_CAPACITY - 1) {

                    printf("last argument = %s\n", argv[j]);
                    packet_len += 3;
                }
            }
            

            packet_len += 1;
            // write the len of the packet
            write(fd_commander, &packet_len, sizeof(int));

            // write the packet in the pipe
            for (int j = index ; j < index + PACKET_CAPACITY ; j++) {
                if (argv[j] == NULL) {
                    write(fd_commander, "\0", sizeof(char));
                    break;
                }
                else if (j == index + PACKET_CAPACITY - 1) {
                    printf("COMMANDER: argv[%d] = %s\n", j, argv[j]);
                    write(fd_commander, argv[j], strlen(argv[j]));
                    write(fd_commander, "\0", sizeof(char));
                }
                else {
                    write(fd_commander, argv[j], strlen(argv[j]));
                    write(fd_commander, " ", sizeof(char));
                }
                    
            }

            // post the semaphore to notify ready to read
            sem_post(serverSem);
            printf("COMMANDER: packet (with index: %d of %d) sent!\n", i+1, packets);

            // wait for the server to read the packet
            sem_wait(commanderSem);
            printf("COMMANDER: packet was received by the server!\n");
            
            // add packet_capacity to the current argument index

            if (broke) {
                index += broke;
                break;
            }
            else {
                index += PACKET_CAPACITY;
            }

        }

        // close the semaphore
        sem_close(serverSem);
    }
    else {
        // write the amount of packets needed to send is one, so no multiple packets
        packets = 1;
        write(fd_commander, &packets, sizeof(int));

        // // write the number of strings for the pipe
        // write(fd_commander, &argc, sizeof(int));

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
        
        // give the signal to jobExecutorServer
        kill(p, SIGUSR1);
    }

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
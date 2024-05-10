jobCommander:
	gcc -o jobCommander jobCommander.c queue.c
	gcc -o jobExecutorServerV2 jobExecutorServerV2.c queue.c

clean:
	rm -f jobCommander
	rm -f jobExecutorServer
	rm -f jobExecutorServer.txt
	rm -f comm
	rm -f comm1
	rm -f server
	rm -f commander
	rm -f jobExecutorServerV2
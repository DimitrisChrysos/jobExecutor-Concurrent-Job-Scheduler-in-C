jobCommander:
	gcc -o jobCommander jobCommander.c
	gcc -o jobExecutorServer jobExecutorServer.c queue.c

clean:
	rm -f jobCommander
	rm -f jobExecutorServer
	rm -f jobExecutorServer.txt
	rm -f comm
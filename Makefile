jobCommander:
	gcc -o jobCommander jobCommander.c queue.c
	gcc -o jobExecutorServer jobExecutorServer.c queue.c ServerCommands.c
	gcc -o progDelay progDelay.c

clean:
	rm -f jobCommander
	rm -f jobExecutorServer
	rm -f jobExecutorServer.txt
	rm -f comm
	rm -f comm1
	rm -f server
	rm -f commander
	rm -f progDelay
	rm -rf my_A_decompressed_directory
	rm -rf my_B_decompressed_directory
	rm -f bFile1.txt
	rm -f bFile2.txt
	rm -f bFile3.txt
	rm -f bFile4.txt
	rm -f bFile5.txt
	rm -f bFile6.txt
	rm -f bFile7.txt
	rm -f bFile8.txt
	rm -f bFile9.txt
	rm -f bFile10.txt
	rm -f my_A_CompressedaFile.tar.gz
	rm -f my_B_CompressedbFile.tar.gz
jobCommander:
	gcc -o jobCommander jobCommander.c queue.c -lpthread
	gcc -o jobExecutorServer jobExecutorServer.c queue.c ServerCommands.c -lpthread
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
	rm -f bFile1.txt bFile2.txt bFile3.txt bFile4.txt bFile5.txt bFile6.txt bFile7.txt bFile8.txt bFile9.txt bFile10.txt
	rm -f ren_aFile1.txt ren_aFile2.txt ren_aFile3.txt ren_aFile4.txt ren_aFile5.txt ren_aFile6.txt ren_aFile7.txt ren_aFile8.txt ren_aFile9.txt ren_aFile10.txt
	rm -f my_A_CompressedaFile.tar.gz
	rm -f my_B_CompressedbFile.tar.gz
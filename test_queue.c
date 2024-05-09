#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"



int main() {
    printf("Created the Queue!\n");
    Queue* myqueue = createQueue(5);

    printf("Insert mytriplet at the Queue!\n");
    Triplet* mytriplet = init_triplet(12, "wget", 2);
    enqueue(myqueue, mytriplet);

    printf("Insert mytriplet1 at the Queue!\n");
    Triplet* mytriplet1 = init_triplet(23, "cool", 123);
    enqueue(myqueue, mytriplet1);


    print_queue_and_stats(myqueue);

    printf("\n");
    printf("Remove first item from the Queue... ");
    Triplet* removed = dequeue(myqueue);
    printf("Removed job \"%s\" from the Queue!\n", removed->job);

    print_queue_and_stats(myqueue);

    delete_triplet(mytriplet);
    delete_triplet(mytriplet1);
    deleteQueue(myqueue);
}
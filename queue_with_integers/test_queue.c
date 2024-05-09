#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int main() {
    printf("Created the Queue!\n");
    Queue* myqueue = createQueue(5);
    printf("Insert 5 at the Queue!\n");
    enqueue(myqueue, 5);
    printf("Insert 6 at the Queue!\n");
    enqueue(myqueue, 6);

    printf("\n");
    printf("Remove first item from the Queue... ");
    printf("Removed %d from the Queue!\n", dequeue(myqueue));

    print_queue_and_stats(myqueue);

    printf("Insert 12 at the Queue!\n");
    enqueue(myqueue, 12);
    printf("Insert 3213 at the Queue!\n");
    enqueue(myqueue, 3213);

    print_queue_and_stats(myqueue);

    printf("\n");
    printf("Remove first item from the Queue... ");
    printf("Removed %d from the Queue!\n", dequeue(myqueue));

    printf("\n");
    printf("Remove first item from the Queue... ");
    printf("Removed %d from the Queue!\n", dequeue(myqueue));

    printf("\n");
    printf("Remove first item from the Queue... ");
    printf("Removed %d from the Queue!\n", dequeue(myqueue));

    printf("\n");
    print_queue_and_stats(myqueue);

    deleteQueue(myqueue);
}
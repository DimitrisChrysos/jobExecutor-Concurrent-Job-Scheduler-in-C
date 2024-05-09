#include <stdio.h>
#include <stdlib.h>
#include "queue.h"


Queue* createQueue() {
    Queue* myqueue = (Queue*)malloc(sizeof(Queue));
    myqueue->size = 0;
    myqueue->first_node = NULL;
    myqueue->last_node = NULL;
}

void deleteQueue(Queue* myqueue) {
    Node* temp_node = myqueue->first_node;
    for (int i = 0 ; i < myqueue->size ; i++) {
        Node* old_node = temp_node;
        temp_node = temp_node->child;
        free(old_node);
    }
    free(myqueue);
}

void set_node(Node* new_node, int value, Node* parent) {
    new_node->value = value;
    new_node->parent = parent;
    new_node->child = NULL;
    if (parent != NULL) {
        parent->child = new_node;
    }
}

int enqueue(Queue* myqueue, int value) {

    if (myqueue->size == 0) {   // if queue is empty
        Node* first_node = (Node*)malloc(sizeof(Node));
        set_node(first_node, value, NULL);
        myqueue->first_node = first_node;
        myqueue->last_node = first_node;
        myqueue->size += 1;
        return 1;
    }
    else {
        Node* new_node = (Node*)malloc(sizeof(Node));
        set_node(new_node, value, myqueue->last_node);
        myqueue->last_node = new_node;
        myqueue->size++;
    }
}

int dequeue(Queue* myqueue) {
    if (myqueue->size == 0) {
        printf("Queue is empty, cannot remove other items\n");
        return -1;
    }
    else if (myqueue->size == 1) {
        int value = myqueue->first_node->value;
        free(myqueue->first_node);
        myqueue->first_node = NULL;
        myqueue->last_node = NULL;
        myqueue->size--;
        return value;
    }
    else {
        int value = myqueue->first_node->value;
        Node* child =  myqueue->first_node->child;
        child->parent = NULL;
        free(myqueue->first_node);
        myqueue->first_node = child;
        myqueue->size--;
        return value;
    }
}

Node* front(Queue* myqueue) {
    return myqueue->first_node;
}

Node* rear(Queue* myqueue) {
    return myqueue->last_node;
}

int queue_empty(Queue myqueue) {
    if (myqueue.size == 0)
        return 1;
    else
        return 0;
}

int queue_size(Queue myqueue) {
    return myqueue.size;
}

void print_queue_and_stats(Queue* myqueue) {
    printf("\n");
    if (queue_empty(*myqueue)) {
        printf("The queue is empty!\n");
        return;
    }
    printf("myqueue: first item value = %d \n", myqueue->first_node->value);
    printf("myqueue: last item value = %d \n", myqueue->last_node->value);
    printf("myqueue: size = %d\n", myqueue->size);

    printf("\n");
    Node* temp_node = myqueue->first_node;
    for (int i = 0 ; i < myqueue->size ; i++) {
        printf("myqueue[%d] = %d\n", i, temp_node->value);
        temp_node = temp_node->child;
    }
    printf("\n");
}
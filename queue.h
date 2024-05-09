typedef struct node Node;

typedef struct node {
    int value;
    int index;
    Node* parent;
    Node* child;
} Node;

typedef struct queue Queue;

typedef struct queue {
    Node* first_node;
    Node* last_node;
    int size;
} Queue;

// creates and initializes the queue
Queue* createQueue();

// deletes the Queue from the memory
void deleteQueue(Queue* myqueue);

// initalizes a node
void set_node(Node* new_node, int value, Node* parent);

// returns 1 if added to queue successfully, returns -1 if not
int enqueue(Queue* myqueue, int value);

// returns 1 if successfully removed, returns -1 if not 
int dequeue(Queue* myqueue);

// returns the first node added to the queue
Node* front(Queue* myqueue);

// returns the last node added to the queue
Node* rear(Queue* myqueue);

// returns 1 if queue empty, else returns 0
int queue_empty(Queue myqueue);

// returns the size of the queue
int queue_size(Queue myqueue);

// prints the whole queue and it's stats
void print_queue_and_stats(Queue* myqueue);
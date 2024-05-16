// struct for the Nodes of the Queue
typedef struct node Node;
typedef struct node {
    void* value;    // value of the node
    int index;  // index of the node
    Node* parent;   // parent of the node
    Node* child;    // child of the node
} Node;

// struct for the Queue
typedef struct queue Queue;
typedef struct queue {
    Node* first_node;   // the first node of the queue
    Node* last_node;    // the last node of the queue
    int size;   // the size of the queue
} Queue;

// creates and initializes the queue
Queue* createQueue();

// deletes the Queue from the memory
void deleteQueue(Queue* myqueue);

// initalizes a node
void set_node(Node* new_node, void* value, Node* parent);

// returns 1 if added to queue successfully, returns -1 if not
int enqueue(Queue* myqueue, void* value);

// returns the value of the node if successfully removed
void* dequeue(Queue* myqueue);

// returns the first node added to the queue
Node* front(Queue* myqueue);

// returns the last node added to the queue
Node* rear(Queue* myqueue);

// returns 1 if queue empty, else returns 0
int queue_empty(Queue myqueue);

// returns the size of the queue
int queue_size(Queue myqueue);

// struct for the Triplet of the jobs
typedef struct triplet Triplet;
typedef struct triplet {
    char* jobID;    // jobID of the Triplet
    char* job;  // job of the Triplet
    int queuePosition;  // the position of the triplet in the queue
    int pid;    // if triplet in the running queue, the pid of the process
} Triplet;

// initalizes a Triplet
Triplet* init_triplet(char* jobID, char* job, int queuePosition, int pid);

// deletes a Triplet
void delete_triplet(Triplet* mytriplet);

// format a Triplet as a string, (memory deallocation is needed!)
char* format_triplet(Triplet* mytriplet);

// prints the whole queue and it's stats (used for debugging)
void print_queue_and_stats(Queue* myqueue);
#include <stdio.h>

#define SIZE 10

void* queue[SIZE];
int Rear = -1, Front = -1; 

// Check if the queue is empty
int isEmpty() {
    return (Front == -1 && Rear == -1);
}

// Check if the queue is full
int isFull() {
    return (Rear == SIZE - 1);
}

// Insert an element into the queue
void enqueue(void* item) {
    if (isFull())
        printf("Overflow!\n");
    else {
        if (isEmpty())
            Front = 0;
        Rear += 1;  // Update rear index of the queue
        queue[Rear] = item;
    }
}

// Remove an element from the queue
void* dequeue() {
    if (isEmpty()) {
        printf("Underflow\n");
        return NULL;
    }

    void* item = queue[Front];
    
    // Restart variables if there's only one item left
    if (Front == Rear) {
        Front = -1;
        Rear = -1;
    }
    else {
        Front = (Front + 1) % SIZE; // Update front index of the queue
    }

    return item;
}
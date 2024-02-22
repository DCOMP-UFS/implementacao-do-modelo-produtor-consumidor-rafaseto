#include <stdio.h>
#define SIZE 10

void* queue[SIZE];
int Rear, Front = -1;

int isEmpty() {
    return (Front == -1); 
}

int isFull() {
    return (Rear == SIZE -1);
}

void enqueue(void* item) {
    if (isFull())
        printf("Overflow!\n");
    else {
        if (isEmpty())
            Front = 0;
        Rear += 1;
        queue[Rear] = item;
    }
}
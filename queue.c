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

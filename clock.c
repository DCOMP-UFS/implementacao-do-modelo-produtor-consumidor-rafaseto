/* File:
 *    clock.c
 *
 * Purpose:
 *    Implementação de uma fila de clocks
 *
 *
 * Compile:  gcc -g -Wall -o clock clock.c -lpthread -lrt
 * Usage:    ./clock PRODUCE_RATE CONSUME_RATE
 * PRODUCE_RATE and CONSUME_RATE optional
 * default: PRODUCE_RATE = 2; CONSUME_RATE = 1;
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define THREAD_NUM 6    // Tamanho do pool de threads
#define BUFFER_SIZE 10 // Númermo máximo de tarefas enfileiradas

int produceRate;
int consumeRate;

typedef struct Clock {
    int p[3];
    long idProducer;
} Clock;

Clock clockQueue[BUFFER_SIZE];
int clockCount = 0;

pthread_mutex_t mutex;

pthread_cond_t condFull;
pthread_cond_t condEmpty;

void consumeClock(Clock *clock, int idConsumer){
    printf("(Consumer Thread %d) Consuming clock {%d, %d, %d} produced by  (Producer Thread %ld)\n", (idConsumer + 1)/2, clock->p[0], clock->p[1], clock->p[2], clock->idProducer);
}

Clock getClock(){
    pthread_mutex_lock(&mutex);

    while (clockCount == 0){
        pthread_cond_wait(&condEmpty, &mutex);
    }

    Clock clock = clockQueue[0];
    int i;
    for (i = 0; i < clockCount - 1; i++){
        clockQueue[i] = clockQueue[i+1];
    }
    clockCount--;

    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&condFull);

    return clock;
}

void submitClock(Clock clock){
    pthread_mutex_lock(&mutex);

    while (clockCount == BUFFER_SIZE){
        pthread_cond_wait(&condFull, &mutex);
    }

    clockQueue[clockCount] = clock;
    clockCount++;

    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&condEmpty);
}

/*-------------------------------------------------------------------*/
void *startConsumerThread(void* args) {
    long id = (long) args;
    while (1){
        Clock clock = getClock();
        consumeClock(&clock, id);
        sleep(consumeRate);
    }
    return NULL;
}

/*-------------------------------------------------------------------*/
void *startProducerThread(void* args) {
    long id = (long) args;
    int myTime = 0;
    while (1){
        Clock clock;
        clock.p[0] = 0;
        clock.p[1] = 0;
        clock.p[2] = 0;
        clock.p[id] = myTime;
        clock.idProducer = (long) id/2 + 1;
        submitClock(clock);
        myTime++;
        sleep(produceRate);
    }
    return NULL;
}

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    if (argc == 1){
        produceRate = 2;
        consumeRate = 1;
    }
    else{
        produceRate = atoi(argv[1]);
        consumeRate = atoi(argv[2]);
    }
    
    printf("Implemented Producer-Consumer model started with\n");
    printf("PRODUCE_RATE = %d; CONSUME_RATE = %d;\n", consumeRate, produceRate);
    // Iniciar semáforo
    pthread_mutex_init(&mutex, NULL);

    pthread_cond_init(&condEmpty, NULL);
    pthread_cond_init(&condFull, NULL);

    //
    pthread_t thread[THREAD_NUM];
    long i;
    for (i = 0; i < THREAD_NUM; i++){
        if (i % 2 == 0){
            if (pthread_create(&thread[i], NULL, &startProducerThread, (void*) i) != 0) {
                perror("Failed to create the thread");
            }
        }
        else{
            if (pthread_create(&thread[i], NULL, &startConsumerThread, (void*) i) != 0) {
                perror("Failed to create the thread");
            }
        }
    }

    for (i = 0; i < THREAD_NUM; i++){
        if (pthread_join(thread[i], NULL) != 0) {
            perror("Failed to join the thread");
        }
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condEmpty);
    pthread_cond_destroy(&condFull);
    return 0;
}  /* main */

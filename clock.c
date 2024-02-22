/**
 * Código base para implementação de relógios vetoriais.
 * Meta: implementar a interação entre três processos ilustrada na figura
 * da URL: 
 * 
 * https://people.cs.rutgers.edu/~pxk/417/notes/images/clocks-vector.png
 * 
 * Compilação: mpicc -o rvet rvet.c
 * Execução:   mpiexec -n 3 ./rvet
 */
 
#include <stdio.h>
#include <string.h>  
#include <mpi.h>
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
// #include <time.h>

#define THREAD_NUM 6    // Tamanho do pool de threads
#define BUFFER_SIZE 256 // Númermo máximo de tarefas enfileiradas   

// Global variables
int clockCount = 0;
Clock clockQueue[BUFFER_SIZE];

pthread_mutex_t mutex;
pthread_cond_t isFull;
pthread_cond_t isEmpty;


typedef struct Clock { 
   int p[3];
} Clock;

// outputs to the console the updated change to a given clock upon an event 
void Clock_logging(int pid, Clock *clock){
   printf("Process: %d, Clock: (%d, %d, %d)\n", pid, clock->p[0], clock->p[1], clock->p[2]);
}

// A queue of clocks waits to be updated upon triggering an event
// Head of queue pops out once event is done
void Event(int pid, Clock *clock, int logg){
   clock->p[pid]++;
   
   // The following condition checks whether logging of the clock is necessary
   // when logg is non-zero (true) i.e., an Event might not necessarily change the current clock
   if (logg) {Clock_logging(pid, clock);}
}

// Given the id of a sender, effectively sends the message to be captured by the receiver
/*
void Send(int pid_send_to, Clock *clock, int pid_sender){
   Event(pid_sender, clock, 0);
   MPI_Send(&clock->p[0], 3, MPI_INT, pid_send_to, 0, MPI_COMM_WORLD);
   Clock_logging(pid_sender, clock);
}

// Given the id of a sender, sets off an event that may or may not be logged
void Receive(int pid_receive_from, Clock *clock, int pid_receiver){
   Clock temp_clock = {{0,0,0}};
   MPI_Status status;
   
   Event(pid_receiver, clock, 0);
   
   MPI_Recv(&temp_clock.p[0], 3, MPI_INT, pid_receive_from, 0, MPI_COMM_WORLD, &status);
   
   //Atualizar o relogio interno comparando com o relogio recebido
   for (int i = 0; i < 3; i++){
      clock->p[i] = (temp_clock.p[i] > clock->p[i]) ? temp_clock.p[i] : clock->p[i];
   }
   Clock_logging(pid_receiver, clock);
}
*/

Clock getClock(){
   pthread_mutex_lock(&mutex);
   
   while (clockCount == 0){
      pthread_cond_wait(&isEmpty, &mutex);
   }
   
   Clock clock = taskQueue[0];
   int i;
   for (i = 0; i < clockCount - 1; i++){
      clockQueue[i] = clockQueue[i+1];
   }
   clockQueue--;
   
   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&isFull);

   return clock;
}

void submitClock(Clock clock){
   pthread_mutex_lock(&mutex);

   while (clockCount == BUFFER_SIZE){
      pthread_cond_wait(&isFull, &mutex);
   }

   clockQueue[clockCount] = clock;
   clockCount++;

   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&isEmpty);
}

void *startThread(void* args);  


// Compacts the generation of processes
// Defines each process according to its ranking

void Call_process(int my_id){
    Clock clock = {{0,0,0}};
    
    switch(my_id){
        case 0:
            Event(my_id, &clock, 1);
            Send(1, &clock, my_id);
            Receive(1, &clock, my_id);
            Send(2, &clock, my_id);
            Receive(2, &clock, my_id);
            Send(1, &clock, my_id);
            Event(my_id, &clock, 1);
            break;
        
        case 1:
            Send(0, &clock, my_id);
            Receive(0, &clock, my_id);
            Receive(0, &clock, my_id);
            break;
        
        case 2:
            Event(my_id, &clock, 1);
            Send(0, &clock, my_id);
            Receive(0, &clock, my_id);
            break;
        
        default:
            perror("The generated process exceeds the number of processes required!!\n");
            printf("Bug found in source code: 404");
            break;
            
            
    }
}

int main(void) {
   int my_rank;               

   MPI_Init(NULL, NULL); 
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

   if (my_rank == 0) { 
      Call_process(0);
   } else if (my_rank == 1) {
      Call_process(1);
   } else if (my_rank == 2) {  
      Call_process(2);
   }

   /* Finaliza MPI */
   MPI_Finalize(); 

   return 0;
}  /* main */

void *startThread(void* args) {
   long id = (long) args; 
   while (1){ 
      Clock clock = getClock();
      Call_process(id);
      sleep(rand()%5);
   }
   return NULL;
} 

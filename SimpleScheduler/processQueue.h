#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

#define MAX_JOBS 100
#define MAX_INPUT_SIZE 1024

struct Job{
    char name[MAX_INPUT_SIZE];
    pid_t pid;
    int waitTimeMillisecs;
    time_t startTime;
    time_t EndTime;
    int isRunnning;
};



// Define a simple queue for ready Jobs (you might want a more sophisticated data structure).
struct Queue {
    struct Job Jobs[100];
    int front;
    int rear;
};
struct Queue readyQueue;

struct shm_queue{
    struct Queue readyQueue;
    sem_t mutex;
    sem_t count;
};
struct shm_queue* rrq;
int rq;

void InitialiseQueue(struct Queue* q){
    q->front = q->rear = -1;
    sem_init(&rrq->mutex,1,1);
    sem_init(&rrq->mutex,1,0);
}

void enqueue(struct Queue* queue, struct Job Job) {
    if (queue->rear == 99) {
        printf("Queue is full.\n");
        return;
    }
    sem_wait(&rrq->mutex);
    queue->Jobs[++queue->rear] = Job;
    sem_post(&rrq->count);
    sem_post(&rrq->count); 
}

struct Job dequeue(struct Queue* queue) {
    if (queue->front == queue->rear) {
        struct Job dummy;
        return dummy; // Queue is empty
    }
    sem_wait(&rrq->count);  // Wait if the queue is empty
    sem_wait(&rrq->mutex);
    struct Job ret = queue->Jobs[++queue->front];
    sem_post(&rrq->mutex);
    return ret;
}

// Initialize the ready queue

// Define functions for Job management.

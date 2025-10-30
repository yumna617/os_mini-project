#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void queue_init(TaskQueue *q) {
    q->front = q->rear = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

void queue_push(TaskQueue *q, Task task) {
    Node *newNode = malloc(sizeof(Node));
    if (!newNode) {
        perror("malloc");
        return;
    }
    newNode->task = task;
    newNode->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (!q->rear)
        q->front = q->rear = newNode;
    else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

Task queue_pop(TaskQueue *q) {
    pthread_mutex_lock(&q->lock);
    while (!q->front)
        pthread_cond_wait(&q->not_empty, &q->lock);

    Node *temp = q->front;
    Task task = temp->task;
    q->front = q->front->next;
    if (!q->front) q->rear = NULL;
    free(temp);

    pthread_mutex_unlock(&q->lock);
    return task;
}

/* ------------------ ClientQueue implementation ------------------ */

void client_queue_init(ClientQueue *q) {
    q->front = q->rear = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

void client_queue_push(ClientQueue *q, int client_fd) {
    ClientNode *newNode = malloc(sizeof(ClientNode));
    if (!newNode) {
        perror("malloc");
        return;
    }
    newNode->client_fd = client_fd;
    newNode->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (!q->rear)
        q->front = q->rear = newNode;
    else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

int client_queue_pop(ClientQueue *q) {
    pthread_mutex_lock(&q->lock);
    while (!q->front)
        pthread_cond_wait(&q->not_empty, &q->lock);

    ClientNode *temp = q->front;
    int fd = temp->client_fd;
    q->front = q->front->next;
    if (!q->front) q->rear = NULL;
    free(temp);

    pthread_mutex_unlock(&q->lock);
    return fd;
}


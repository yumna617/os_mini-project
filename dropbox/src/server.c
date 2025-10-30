#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define CLIENT_THREADS 4
#define WORKER_THREADS 4

TaskQueue taskQueue;
ClientQueue clientQueue; /* new client queue */

static volatile int server_fd = -1;

/* Forward declaration (client_thread is implemented in client_handler.c) */
void *client_thread(void *arg);

void *client_pool_worker(void *arg) {
    (void)arg;
    while (1) {
        int client_fd = client_queue_pop(&clientQueue);
        /* client_thread expects a malloc'd int* and frees it when done.
           Allocate and pass a pointer; call client_thread as a function
           so the pool thread does the I/O for that client session (no new
           per-connection thread is created). */
        int *pclient = malloc(sizeof(int));
        if (!pclient) {
            perror("malloc");
            close(client_fd);
            continue;
        }
        *pclient = client_fd;
        /* call client_thread directly (it will free pclient when done) */
        client_thread(pclient);
        /* when client_thread returns, it already closed the client socket */
    }
    return NULL;
}

void shutdown_handler(int sig) {
    (void)sig;
    if (server_fd >= 0) close(server_fd);
    printf("Server shutting down.\n");
    exit(0);
}

int main() {
    signal(SIGINT, shutdown_handler);
    init_metadata();
    queue_init(&taskQueue);
    client_queue_init(&clientQueue);

    pthread_t workers[WORKER_THREADS];
    for (int i = 0; i < WORKER_THREADS; i++) {
        if (pthread_create(&workers[i], NULL, worker_thread, NULL) != 0) {
            perror("pthread_create worker");
            exit(1);
        }
    }

    /* create client threadpool */
    pthread_t client_workers[CLIENT_THREADS];
    for (int i = 0; i < CLIENT_THREADS; i++) {
        if (pthread_create(&client_workers[i], NULL, client_pool_worker, NULL) != 0) {
            perror("pthread_create client pool");
            exit(1);
        }
    }

    int new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Server running on port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        /* push accepted socket into client queue for pool threads to handle */
        client_queue_push(&clientQueue, new_socket);
    }

    close(server_fd);
    return 0;
}


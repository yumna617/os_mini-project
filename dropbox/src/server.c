#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>


#define CLIENT_THREADS 4
#define WORKER_THREADS 4


TaskQueue clientQueue;
TaskQueue taskQueue;


int main() {
init_metadata();
queue_init(&clientQueue);
queue_init(&taskQueue);


pthread_t workers[WORKER_THREADS];
for (int i = 0; i < WORKER_THREADS; i++)
pthread_create(&workers[i], NULL, worker_thread, NULL);


int server_fd, new_socket;
struct sockaddr_in address;
socklen_t addrlen = sizeof(address);


server_fd = socket(AF_INET, SOCK_STREAM, 0);
address.sin_family = AF_INET;
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(PORT);


bind(server_fd, (struct sockaddr *)&address, sizeof(address));
listen(server_fd, BACKLOG);
printf("Server running on port %d...\n", PORT);


while (1) {
new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
if (new_socket < 0) continue;
int *pclient = malloc(sizeof(int));
*pclient = new_socket;
pthread_t tid;
pthread_create(&tid, NULL, client_thread, pclient);
pthread_detach(tid);
}


close(server_fd);
return 0;
}

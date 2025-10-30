#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include <stdbool.h>
#include <netinet/in.h>

#define MAX_FILENAME 256
#define MAX_USER 64
#define MAX_PATH 512
#define PORT 8080
#define BACKLOG 10

typedef enum {
    CMD_UPLOAD,
    CMD_DOWNLOAD,
    CMD_DELETE,
    CMD_LIST
} CommandType;

typedef struct {
    int client_fd;
    char username[MAX_USER];
    CommandType cmd;
    char filename[MAX_FILENAME];
} Task;

typedef struct Node {
    Task task;
    struct Node *next;
} Node;

typedef struct {
    Node *front, *rear;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
} TaskQueue;

/* Client queue (thread-safe queue of int socket descriptors) */
typedef struct ClientNode {
    int client_fd;
    struct ClientNode *next;
} ClientNode;

typedef struct {
    ClientNode *front, *rear;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
} ClientQueue;

typedef struct {
    char filename[MAX_FILENAME];
    long size;
} FileMeta;

typedef struct {
    char username[MAX_USER];
    long quota_limit;
    long quota_used;
} UserMeta;

/* Queue functions for TaskQueue (already used by workers) */
void queue_init(TaskQueue *q);
void queue_push(TaskQueue *q, Task task);
Task queue_pop(TaskQueue *q);

/* Client queue functions (new) */
void client_queue_init(ClientQueue *q);
void client_queue_push(ClientQueue *q, int client_fd);
int client_queue_pop(ClientQueue *q);

void init_metadata();
void add_file_metadata(const char *user, const char *filename, long size);
void add_user_file(const char *username, const char *filename);
void remove_user_file(const char* username, const char* filename);


void list_user_files(int client_fd, const char *user);

void *worker_thread(void *arg);
void *client_thread(void *arg);

#endif


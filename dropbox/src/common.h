#ifndef COMMON_H


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


typedef struct {
char filename[MAX_FILENAME];
long size;
} FileMeta;


typedef struct {
char username[MAX_USER];
long quota_limit;
long quota_used;
} UserMeta;


// ------------- FUNCTION DECLARATIONS -------------
void queue_init(TaskQueue *q);
void queue_push(TaskQueue *q, Task task);
Task queue_pop(TaskQueue *q);


// Metadata functions
void init_metadata();
void add_file_metadata(const char *user, const char *filename, long size);
void add_user_file(const char *username, const char *filename);
void remove_user_file(const char* username, const char* filename);
void list_user_files(int client_fd, const char *user);


// Worker and Client Handlers
void *worker_thread(void *arg);
void *client_thread(void *arg);


#endif

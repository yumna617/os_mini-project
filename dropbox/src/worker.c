#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern TaskQueue taskQueue;

void upload_file(Task task) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "server_storage/%s_%s", task.username, task.filename);
    FILE *f = fopen(path, "wb");
    if (!f) return;
    char buffer[1024];
    int bytes;
    while ((bytes = recv(task.client_fd, buffer, sizeof(buffer), 0)) > 0) {
        if (strncmp(buffer, "EOF", 3) == 0) break;
        fwrite(buffer, 1, bytes, f);
    }
    fclose(f);

    add_user_file(task.username, task.filename);
}

void delete_file(Task task) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "server_storage/%s_%s", task.username, task.filename);

    // Physically remove file
    if (remove(path) == 0) {
        // ✅ Also remove from metadata
        remove_user_file(task.username, task.filename);
        printf("[INFO] Deleted file %s for user %s\n", task.filename, task.username);
    } else {
        perror("[ERROR] Deletion failed");
    }
}

void *worker_thread(void *arg) {
    while (1) {
        Task t = queue_pop(&taskQueue);
        switch (t.cmd) {
            case CMD_UPLOAD:
                upload_file(t);
                break;

            case CMD_DOWNLOAD:
                break;

            case CMD_DELETE:
                delete_file(t);
                break;

            case CMD_LIST:
                list_user_files(t.client_fd, t.username);
                break;
        }
    }
    return NULL;
}


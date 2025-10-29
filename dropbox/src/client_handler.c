#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

extern TaskQueue taskQueue;

void *client_thread(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char username[MAX_USER];
    send(client_fd, "Enter username: ", 17, 0);
    recv(client_fd, username, sizeof(username), 0);
    username[strcspn(username, "\n")] = 0;

    send(client_fd, "Commands: UPLOAD <file>, DELETE <file>, LIST, QUIT\n", 51, 0);

    char buffer[256];
    char cmd[64];
    char filename[MAX_FILENAME];
    ssize_t bytes_read;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0)
            break;

        buffer[bytes_read] = '\0';
        buffer[strcspn(buffer, "\r\n")] = 0;

        printf("[DEBUG] Command from %s: '%s'\n", username, buffer);

        if (strlen(buffer) == 0)
            continue;

        if (sscanf(buffer, "%s %s", cmd, filename) < 1)
            continue;

        if (strcasecmp(cmd, "UPLOAD") == 0) {
            char path[MAX_PATH];
            snprintf(path, sizeof(path), "server_storage/%s_%s", username, filename);

            FILE *f = fopen(path, "wb");
            if (!f) {
                send(client_fd, "Error: could not save file.\n", 29, 0);
                continue;
            }

            char filebuf[1024];
            ssize_t n;
            while ((n = recv(client_fd, filebuf, sizeof(filebuf), 0)) > 0) {
                if (n >= 3 && strncmp(filebuf, "EOF", 3) == 0)
                    break;
                fwrite(filebuf, 1, n, f);
                if (n < (ssize_t)sizeof(filebuf)) break; // safety: if recv returned less than buffer, assume end for this implementation
            }
            fclose(f);

            add_user_file(username, filename);
            send(client_fd, "File uploaded successfully.\n", 29, 0);
        }

        else if (strcasecmp(cmd, "DELETE") == 0) {
            char path[MAX_PATH];
            snprintf(path, sizeof(path), "server_storage/%s_%s", username, filename);
            if (remove(path) == 0) {
                // remove from in-memory metadata as well
                remove_user_file(username, filename);
                send(client_fd, "File deleted successfully.\n", 27, 0);
            } else {
                send(client_fd, "File not found.\n", 16, 0);
            }
        }

        else if (strcasecmp(cmd, "LIST") == 0) {
            list_user_files(client_fd, username);
        }

        else if (strcasecmp(cmd, "QUIT") == 0) {
            send(client_fd, "Goodbye!\n", 9, 0);
            break;
        }

        else {
            send(client_fd, "Unknown command.\n", 17, 0);
        }
    }

    close(client_fd);
    return NULL;
}


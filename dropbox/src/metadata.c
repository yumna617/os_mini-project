#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 10
#define MAX_FILES_PER_USER 100

typedef struct {
    char username[64];
    char files[MAX_FILES_PER_USER][128];
    int file_count;
} UserMetadata;

static UserMetadata users[MAX_USERS];
static int user_count = 0;

void init_metadata() {
    system("mkdir -p server_storage");
}

UserMetadata* get_user(const char* username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0)
            return &users[i];
    }
    // Ensure we don't overflow
    if (user_count >= MAX_USERS) return NULL;
    strcpy(users[user_count].username, username);
    users[user_count].file_count = 0;
    return &users[user_count++];
}

void add_user_file(const char* username, const char* filename) {
    UserMetadata* u = get_user(username);
    if (!u) return;
    // avoid duplicates
    for (int i = 0; i < u->file_count; ++i) {
        if (strcmp(u->files[i], filename) == 0) return;
    }
    if (u->file_count < MAX_FILES_PER_USER) {
        strcpy(u->files[u->file_count++], filename);
    }
}

void remove_user_file(const char* username, const char* filename) {
    UserMetadata* u = get_user(username);
    if (!u) return;
    for (int i = 0; i < u->file_count; i++) {
        if (strcmp(u->files[i], filename) == 0) {
            // Shift all remaining files up one slot
            for (int j = i; j < u->file_count - 1; j++) {
                strcpy(u->files[j], u->files[j + 1]);
            }
            u->file_count--;
            break;
        }
    }
}

void list_user_files(int client_fd, const char* username) {
    UserMetadata* u = get_user(username);
    if (!u || u->file_count == 0) {
        send(client_fd, "No files found.\n", 16, 0);
        return;
    }
    char buffer[1024] = "";
    for (int i = 0; i < u->file_count; i++) {
        strcat(buffer, u->files[i]);
        strcat(buffer, "\n");
    }
    send(client_fd, buffer, strlen(buffer), 0);
}


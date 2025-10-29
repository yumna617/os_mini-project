#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void send_file_data(int sock, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("❌ File not found: %s\n", filename);
        return;
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        send(sock, buffer, bytes, 0);
    }

    // Signal end of file transfer
    send(sock, "EOF", 3, 0);
    fclose(f);
    printf("✅ File '%s' uploaded successfully.\n", filename);
}

int main() {
    int sock;
    struct sockaddr_in server;
    char buffer[1024];
    char command[256];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to Dropbox server on 127.0.0.1:8080\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        printf("%s", buffer);

        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        // Extract command and filename
        char cmd[64], filename[256];
        sscanf(command, "%s %s", cmd, filename);

        // Send the main command to server
        send(sock, command, strlen(command), 0);

        // If it's an upload, send file contents too
        if (strcasecmp(cmd, "UPLOAD") == 0) {
            send_file_data(sock, filename);
        }

        if (strncmp(command, "QUIT", 4) == 0)
            break;
    }

    close(sock);
    return 0;
}


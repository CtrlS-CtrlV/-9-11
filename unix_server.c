/* unix_server.c — UNIX-domain socket server */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>

#define SOCK_PATH "/tmp/ipc_unix_sock"
#define BUF_SIZE  256

int main(void)
{
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCK_PATH);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("[Server] Очікую підключення (%s)...\n", SOCK_PATH);

    for (int client_num = 0; client_num < 3; client_num++) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("[Server] Клієнт #%d підключився.\n", client_num + 1);

        char buf[BUF_SIZE];
        ssize_t n;

        while ((n = recv(client_fd, buf, sizeof(buf) - 1, 0)) > 0) {
            buf[n] = '\0';
            printf("[Server] Запит: \"%s\"\n", buf);

            char response[BUF_SIZE];

            if (strcmp(buf, "TIME") == 0) {
                time_t now = time(NULL);
                snprintf(response, sizeof(response), "%s", ctime(&now));
            } else if (strncmp(buf, "ECHO ", 5) == 0) {
                snprintf(response, sizeof(response), "ECHO: %s", buf + 5);
            } else if (strcmp(buf, "QUIT") == 0) {
                snprintf(response, sizeof(response), "BYE");
                send(client_fd, response, strlen(response), 0);
                break;
            } else {
                snprintf(response, sizeof(response), "ERR: невідома команда '%s'", buf);
            }

            send(client_fd, response, strlen(response), 0);
        }

        close(client_fd);
        printf("[Server] Клієнт #%d відключився.\n", client_num + 1);
    }

    close(server_fd);
    unlink(SOCK_PATH);
    printf("[Server] Завершено.\n");

    return EXIT_SUCCESS;
}

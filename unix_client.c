/* unix_client.c — UNIX-domain socket client */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/ipc_unix_sock"
#define BUF_SIZE  256

int main(void)
{
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    printf("[Client] Підключено до сервера.\n");

    const char *commands[] = { "TIME", "ECHO Привіт", "ECHO IPC test", "QUIT" };
    char buf[BUF_SIZE];

    for (int i = 0; i < 4; i++) {
        printf("[Client] Надсилаю: %s\n", commands[i]);
        send(sock_fd, commands[i], strlen(commands[i]), 0);

        ssize_t n = recv(sock_fd, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = '\0';
            printf("[Client] Відповідь: %s\n", buf);
        }

        if (strcmp(commands[i], "QUIT") == 0) break;
    }

    close(sock_fd);
    printf("[Client] Завершено.\n");

    return EXIT_SUCCESS;
}

/* fifo_server.c — Серверний процес для обміну через FIFO */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_C2S "/tmp/fifo_client_to_server"
#define FIFO_S2C "/tmp/fifo_server_to_client"
#define BUF_SIZE 256

int main(void)
{
    mkfifo(FIFO_C2S, 0666);
    mkfifo(FIFO_S2C, 0666);

    printf("[Server] Очікую підключення клієнта...\n");

    int fd_read = open(FIFO_C2S, O_RDONLY);
    int fd_write = open(FIFO_S2C, O_WRONLY);

    if (fd_read == -1 || fd_write == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    printf("[Server] Клієнт підключений.\n");

    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = read(fd_read, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        printf("[Server] Отримано: \"%s\"\n", buf);

        char response[BUF_SIZE];
        snprintf(response, sizeof(response), "ACK: %s", buf);
        write(fd_write, response, strlen(response));
        printf("[Server] Відправлено: \"%s\"\n", response);
    }

    close(fd_read);
    close(fd_write);
    unlink(FIFO_C2S);
    unlink(FIFO_S2C);
    printf("[Server] Завершено.\n");

    return EXIT_SUCCESS;
}

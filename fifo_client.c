/* fifo_client.c — Клієнтський процес */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_C2S "/tmp/fifo_client_to_server"
#define FIFO_S2C "/tmp/fifo_server_to_client"
#define BUF_SIZE 256

int main(void)
{
    printf("[Client] Підключаюсь до сервера...\n");

    int fd_write = open(FIFO_C2S, O_WRONLY);
    int fd_read  = open(FIFO_S2C, O_RDONLY);

    if (fd_write == -1 || fd_read == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    const char *messages[] = {
        "Привіт, сервер!",
        "IPC через FIFO",
        "Останнє повідомлення",
        NULL
    };

    char buf[BUF_SIZE];

    for (int i = 0; messages[i] != NULL; i++) {
        write(fd_write, messages[i], strlen(messages[i]));
        printf("[Client] Надіслано: \"%s\"\n", messages[i]);

        ssize_t n = read(fd_read, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("[Client] Відповідь: \"%s\"\n", buf);
        }
    }

    close(fd_write);
    close(fd_read);
    printf("[Client] Завершено.\n");

    return EXIT_SUCCESS;
}

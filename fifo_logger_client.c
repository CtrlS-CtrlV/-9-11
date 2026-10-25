/* fifo_logger_client.c — Клієнт логування */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/fifo_log"
#define BUF_SIZE  256

int main(int argc, char *argv[])
{
    int client_id = (argc > 1) ? atoi(argv[1]) : 1;

    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open FIFO");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 5; i++) {
        char msg[BUF_SIZE];
        snprintf(msg, sizeof(msg), "Client-%d: повідомлення #%d", client_id, i + 1);
        write(fd, msg, strlen(msg));
        printf("[Client-%d] Надіслано: %s\n", client_id, msg);
        usleep(100000);
    }

    close(fd);
    return EXIT_SUCCESS;
}

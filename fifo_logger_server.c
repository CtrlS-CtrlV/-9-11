/* fifo_logger_server.c — Сервер логування */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define FIFO_PATH "/tmp/fifo_log"
#define LOG_FILE  "ipc_log.txt"
#define BUF_SIZE  512

int main(void)
{
    mkfifo(FIFO_PATH, 0666);

    printf("[LogServer] Очікую повідомлення (FIFO: %s)...\n", FIFO_PATH);

    FILE *logfile = fopen(LOG_FILE, "a");
    if (!logfile) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        fclose(logfile);
        return EXIT_FAILURE;
    }

    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';

        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

        fprintf(logfile, "[%s] %s\n", timestamp, buf);
        fflush(logfile);
        printf("[LogServer] [%s] %s\n", timestamp, buf);
    }

    close(fd);
    fclose(logfile);
    unlink(FIFO_PATH);
    printf("[LogServer] Завершено. Лог збережено у %s\n", LOG_FILE);

    return EXIT_SUCCESS;
}

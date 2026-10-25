#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>

#define MQ_NAME  "/ipc_demo_mq"
#define MAX_MSG  256

int main(int argc, char *argv[])
{
    int client_id = (argc > 1) ? atoi(argv[1]) : 1;

    mqd_t mq = mq_open(MQ_NAME, O_WRONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 5; i++) {
        char msg[MAX_MSG];
        unsigned int prio = (i % 3);

        snprintf(msg, sizeof(msg),
                 "Client-%d: msg #%d (prio=%u)", client_id, i + 1, prio);

        if (mq_send(mq, msg, strlen(msg), prio) == -1) {
            perror("mq_send");
            break;
        }
        printf("[Client-%d] Надіслано (prio=%u): %s\n", client_id, prio, msg);
    }

    mq_close(mq);
    printf("[Client-%d] Завершено.\n", client_id);

    return EXIT_SUCCESS;
}

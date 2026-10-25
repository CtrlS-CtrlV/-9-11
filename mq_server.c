
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MQ_NAME    "/ipc_demo_mq"
#define MAX_MSG    256
#define MAX_MSGS   10

int main(void)
{
    struct mq_attr attr;
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = MAX_MSGS;
    attr.mq_msgsize = MAX_MSG;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDONLY, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return EXIT_FAILURE;
    }

    printf("[MQ Server] Очікую повідомлення (черга: %s)...\n", MQ_NAME);

    char buf[MAX_MSG + 1];
    unsigned int priority;

    for (int i = 0; i < 15; i++) {
        ssize_t n = mq_receive(mq, buf, MAX_MSG + 1, &priority);
        if (n == -1) {
            perror("mq_receive");
            break;
        }
        buf[n] = '\0';

        printf("[MQ Server] Пріоритет=%u: \"%s\"\n", priority, buf);
    }

    mq_close(mq);
    mq_unlink(MQ_NAME);
    printf("[MQ Server] Завершено.\n");

    return EXIT_SUCCESS;
}

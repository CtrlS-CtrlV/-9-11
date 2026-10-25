/* mq_async.c — Асинхронне отримання через mq_notify */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>

#define MQ_NAME  "/ipc_async_mq"
#define MAX_MSG  256
#define MAX_MSGS 10

static mqd_t mq;
static volatile sig_atomic_t msg_count = 0;

void handle_message(int signo)
{
    (void)signo;
    char buf[MAX_MSG + 1];
    unsigned int prio;

    ssize_t n = mq_receive(mq, buf, MAX_MSG + 1, &prio);
    if (n > 0) {
        buf[n] = '\0';
        printf("[Async] Отримано (prio=%u): \"%s\"\n", prio, buf);
        msg_count++;
    }

    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo  = SIGUSR1;
    mq_notify(mq, &sev);
}

int main(void)
{
    struct mq_attr attr = { .mq_maxmsg = MAX_MSGS, .mq_msgsize = MAX_MSG };

    mq = mq_open(MQ_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return EXIT_FAILURE;
    }

    struct sigaction sa;
    sa.sa_handler = handle_message;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo  = SIGUSR1;
    mq_notify(mq, &sev);

    printf("[Async] Очікую повідомлення (SIGUSR1)...\n");
    printf("[Async] Надсилайте через mq_client. Ctrl+C для виходу.\n");

    while (msg_count < 10) {
        pause();
    }

    mq_close(mq);
    mq_unlink(MQ_NAME);
    printf("[Async] Завершено. Отримано %d повідомлень.\n", msg_count);

    return EXIT_SUCCESS;
}

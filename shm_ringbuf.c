#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>

#define BUF_SLOTS  8
#define NUM_ITEMS  25

typedef struct {
    int    buffer[BUF_SLOTS];
    int    in_idx;
    int    out_idx;
    sem_t  sem_empty;
    sem_t  sem_full;
    sem_t  sem_mutex;
} ring_buf_t;

int main(void)
{
    ring_buf_t *rb = mmap(NULL, sizeof(ring_buf_t),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (rb == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    rb->in_idx = 0;
    rb->out_idx = 0;
    sem_init(&rb->sem_empty, 1, BUF_SLOTS);
    sem_init(&rb->sem_full,  1, 0);
    sem_init(&rb->sem_mutex, 1, 1);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* --- Споживач (дочірній) --- */
        for (int i = 0; i < NUM_ITEMS; i++) {
            sem_wait(&rb->sem_full);
            sem_wait(&rb->sem_mutex);

            int item = rb->buffer[rb->out_idx];
            rb->out_idx = (rb->out_idx + 1) % BUF_SLOTS;

            sem_post(&rb->sem_mutex);
            sem_post(&rb->sem_empty);

            printf("  [Consumer] Прочитано: %d\n", item);
        }
        _exit(EXIT_SUCCESS);
    }

    /* --- Виробник (батько) --- */
    for (int i = 0; i < NUM_ITEMS; i++) {
        sem_wait(&rb->sem_empty);
        sem_wait(&rb->sem_mutex);

        rb->buffer[rb->in_idx] = i * 10;
        printf("  [Producer] Записано: %d у слот %d\n",
               i * 10, rb->in_idx);
        rb->in_idx = (rb->in_idx + 1) % BUF_SLOTS;

        sem_post(&rb->sem_mutex);
        sem_post(&rb->sem_full);
    }

    waitpid(pid, NULL, 0);

    sem_destroy(&rb->sem_empty);
    sem_destroy(&rb->sem_full);
    sem_destroy(&rb->sem_mutex);
    munmap(rb, sizeof(ring_buf_t));

    printf("[main] Завершено.\n");
    return EXIT_SUCCESS;
}

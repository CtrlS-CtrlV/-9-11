#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>

#define SHM_NAME  "/ipc_demo_shm"
#define SEM_WRITE "/ipc_demo_sem_w"
#define SEM_READ  "/ipc_demo_sem_r"

typedef struct {
    int    id;
    double value;
    char   description[128];
} data_record_t;

#define NUM_RECORDS 5

int main(void)
{
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0);
    if (shm_fd == -1) {
        perror("shm_open");
        return EXIT_FAILURE;
    }

    size_t shm_size = sizeof(data_record_t);

    data_record_t *shared = mmap(NULL, shm_size,
                                  PROT_READ,
                                  MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    sem_t *sem_w = sem_open(SEM_WRITE, 0);
    sem_t *sem_r = sem_open(SEM_READ,  0);

    if (sem_w == SEM_FAILED || sem_r == SEM_FAILED) {
        perror("sem_open");
        return EXIT_FAILURE;
    }

    printf("[Reader] Очікую дані...\n");

    for (int i = 0; i < NUM_RECORDS; i++) {
        sem_wait(sem_r);

        printf("[Reader] Прочитано: id=%d, value=%.2f, desc=\"%s\"\n",
               shared->id, shared->value, shared->description);

        sem_post(sem_w);
    }

    munmap(shared, shm_size);
    close(shm_fd);
    sem_close(sem_w);
    sem_close(sem_r);

    printf("[Reader] Завершено.\n");
    return EXIT_SUCCESS;
}

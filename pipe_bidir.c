/* pipe_bidir.c — Двонаправлена комунікація батько↔дочірній через два pipe */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define ARRAY_SIZE 10

int main(void)
{
    int pipe_parent_to_child[2];  /* батько → дочірній */
    int pipe_child_to_parent[2];  /* дочірній → батько */

    if (pipe(pipe_parent_to_child) == -1 || pipe(pipe_child_to_parent) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    int data[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (i + 1) * 10;  /* 10, 20, 30, ..., 100 */
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* --- Дочірній процес --- */
        close(pipe_parent_to_child[1]);  /* закрити запис (батько→дочірній) */
        close(pipe_child_to_parent[0]);  /* закрити читання (дочірній→батько) */

        int received[ARRAY_SIZE];
        read(pipe_parent_to_child[0], received, sizeof(received));

        long sum = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            sum += received[i];
        }
        printf("[Дочірній PID=%d] Отримано масив, сума = %ld\n",
               getpid(), sum);

        write(pipe_child_to_parent[1], &sum, sizeof(sum));

        close(pipe_parent_to_child[0]);
        close(pipe_child_to_parent[1]);
        _exit(EXIT_SUCCESS);
    }

    /* --- Батьківський процес --- */
    close(pipe_parent_to_child[0]);  /* закрити читання */
    close(pipe_child_to_parent[1]);  /* закрити запис */

    printf("[Батько PID=%d] Надсилаю масив...\n", getpid());
    write(pipe_parent_to_child[1], data, sizeof(data));
    close(pipe_parent_to_child[1]);

    long result;
    read(pipe_child_to_parent[0], &result, sizeof(result));
    printf("[Батько] Отримана сума від дочірнього: %ld\n", result);

    long expected = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) expected += data[i];
    printf("[Батько] Перевірка: %ld %s\n", expected,
           (result == expected) ? "— збіг" : "— ПОМИЛКА");

    close(pipe_child_to_parent[0]);
    waitpid(pid, NULL, 0);

    return EXIT_SUCCESS;
}

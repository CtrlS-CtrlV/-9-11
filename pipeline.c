/* pipeline.c — Конвеєр трьох процесів через pipe + dup2 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int pipe1[2];  /* generator → filter */
    int pipe2[2];  /* filter → counter */

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]);

        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);

        for (int i = 1; i <= 100; i++) {
            printf("%d\n", i);
        }

        _exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);

        close(pipe1[0]);
        close(pipe2[1]);

        int num;
        while (scanf("%d", &num) == 1) {
            if (num % 2 == 0) {
                printf("%d\n", num);
            }
        }

        _exit(EXIT_SUCCESS);
    }

    pid_t pid3 = fork();
    if (pid3 == 0) {
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);

        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);

        int num, count = 0;
        while (scanf("%d", &num) == 1) {
            count++;
        }
        fprintf(stderr, "[Counter] Парних чисел: %d\n", count);

        _exit(EXIT_SUCCESS);
    }

    close(pipe1[0]); close(pipe1[1]);
    close(pipe2[0]); close(pipe2[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return EXIT_SUCCESS;
}

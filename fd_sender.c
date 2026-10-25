#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/ipc_fd_pass"
#define TEST_FILE "secret_test.txt"

int send_fd(int socket, int fd_to_send)
{
    struct msghdr msg = {0};
    struct iovec iov;
    char buf[1] = {'F'};

    iov.iov_base = buf;
    iov.iov_len  = 1;
    msg.msg_iov    = &iov;
    msg.msg_iovlen = 1;

    char cmsg_buf[CMSG_SPACE(sizeof(int))];
    msg.msg_control    = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type  = SCM_RIGHTS;
    cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
    memcpy(CMSG_DATA(cmsg), &fd_to_send, sizeof(int));

    return sendmsg(socket, &msg, 0);
}

int main(void)
{
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCK_PATH);
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 1);

    printf("[Sender] Створюю тестовий файл та очікую клієнта...\n");
    int file_fd = open(TEST_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    write(file_fd, "Це секретне повідомлення передане через дескриптор!", 51);
    close(file_fd);

    int client_sock = accept(server_fd, NULL, NULL);

    int fd_to_pass = open(TEST_FILE, O_RDONLY);
    send_fd(client_sock, fd_to_pass);
    close(fd_to_pass);

    printf("[Sender] Файловий дескриптор успішно передано клієнту!\n");

    close(client_sock);
    close(server_fd);
    unlink(SOCK_PATH);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/ipc_fd_pass"

int recv_fd(int socket)
{
    struct msghdr msg = {0};
    struct iovec iov;
    char buf[1];

    iov.iov_base = buf;
    iov.iov_len  = 1;
    msg.msg_iov    = &iov;
    msg.msg_iovlen = 1;

    char cmsg_buf[CMSG_SPACE(sizeof(int))];
    msg.msg_control    = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);

    if (recvmsg(socket, &msg, 0) < 0) return -1;

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        int recv_fd_val;
        memcpy(&recv_fd_val, CMSG_DATA(cmsg), sizeof(int));
        return recv_fd_val;
    }
    return -1;
}

int main(void)
{
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    while (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        usleep(100000);
    }

    printf("[Receiver] Підключено до відправника. Отримую файловий дескриптор...\n");
    int received_fd = recv_fd(sock_fd);

    if (received_fd != -1) {
        char file_buf[128];
        ssize_t bytes = read(received_fd, file_buf, sizeof(file_buf) - 1);
        if (bytes > 0) {
            file_buf[bytes] = '\0';
            printf("[Receiver] Успішно прочитано з отриманого файлового дескриптора:\n\"%s\"\n", file_buf);
        }
        close(received_fd);
    } else {
        printf("[Receiver] Помилка отримання дескриптора.\n");
    }

    close(sock_fd);
    return 0;
}

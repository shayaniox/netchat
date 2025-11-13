#include "box.h"
#include "check.h"
#include "estring.h"
#include "log.h"
#include "tutil.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

int server_message(struct box *srv_box, char *buf, size_t buflen);

int main(void)
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sfd >= 0, "socket");

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    assert(
        connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != -1,
        "connect");

    struct pollfd pfds[2] = {0};
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = sfd;
    pfds[1].events = POLLIN | POLLHUP;

    int nread, nserv;
    char stdin_buf[1024] = {0};
    char servr_buf[1024] = {0};

    int row, column;
    get_cursor_pos(&row, &column);

    string_t str = str_init();
    struct box *svr_box = box_init(row, column, str);

    while (1) {
        int ready = poll(pfds, 2, -1);
        if (ready == -1) {
            if (errno == EINTR)
                continue;
            err_exit("failed on poll syscall");
        }

        if (pfds[0].revents & POLLIN) {
            nread = read(STDIN_FILENO, stdin_buf, sizeof(stdin_buf) - 1);
            if (nread == -1) {
                close(sfd);
                err_exit("read from stdin");
            }
            if (stdin_buf[0] != 0 && send(sfd, stdin_buf, nread, MSG_DONTWAIT) == -1) {
                if (!(errno == EAGAIN || errno == EWOULDBLOCK))
                    error("write to socket");
            }
            memset(stdin_buf, 0, nread);
        }

        if (pfds[1].revents & POLLIN) {
            nserv = read(sfd, servr_buf, sizeof(servr_buf) - 1);
            if (nserv == 0) {
                close(sfd);
                break;
            }
            if (nserv == -1) {
                close(sfd);
                err_exit("read from socket");
            }
            char *nl = strchr(servr_buf, '\n');
            if (nl) {
                *nl = '\0';
                nserv -= 1;
            }

            server_message(svr_box, servr_buf, nserv);
            memset(servr_buf, 0, nserv);
        }
        if (pfds[1].revents & POLLHUP) {
            close(sfd);
            break;
        }
    }

    return EXIT_SUCCESS;
}

int server_message(struct box *srv_box, char *buf, size_t buflen)
{
    if (srv_box == NULL)
        return EINVAL;

    int row, column;
    get_cursor_pos(&row, &column);

    srv_box->row = row;
    srv_box->column = column;
    str_set(srv_box->text, buf, buflen);

    shift(3);

    move_cursor(row, 1);
    box_draw(srv_box);
    move_cursor(row + 3, column);

    return 0;
}

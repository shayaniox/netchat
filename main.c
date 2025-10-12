#include "check.h"
#include "list.h"
#include "log.h"
#include "plist.h"
#include "termutil.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

ssize_t readline(int cfd, char *str, size_t n);

int main(void)
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sfd >= 0, "failed to create a socket file");

    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    int port = 8080;
    struct sockaddr_in sa;
    sa.sin_port = ntohs(port);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int result = bind(sfd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
    assert(result != -1, "failed to bind socket to an address");

    char addrstr[INET_ADDRSTRLEN] = {0};
    assert(inet_ntop(AF_INET, (void *)&sa.sin_addr, addrstr, INET_ADDRSTRLEN) != NULL, "failed to conver the address to string");

    info("Serving the server on: %s:%d", addrstr, port);

    listen(sfd, 10);
    struct plist *pl = pl_init(32);
    assert(pl != NULL, "failed to initialize poll list");

    pl->data[0].fd = sfd;
    pl->data[0].events = POLLIN;
    pl->data[1].fd = STDIN_FILENO;
    pl->data[1].events = POLLIN;

    char buf[64] = {0};
    while (1) {
        int ready = poll(pl->data, pl->len, -1);
        if (ready == -1) {
            error("failed on poll syscall");
            continue;
        }

        if (pl->data[0].revents & POLLIN) {
            // ----------- Handle Client Messages ----------------
            info("A new client connected");

            int cfd = accept(sfd, NULL, NULL);
            if (readline(cfd, buf, 64) == -1) {
                error("failed to read line from client socket");
                goto conn;
            }

            char *nl = strchr(buf, '\n');
            if (nl) *nl = '\0';

            info("received message: %s", buf);
            if (send(cfd, "OK\n", 3, 0) == -1) {
                error("failed to write to client socket");
                goto conn;
            }
        conn:
            close(cfd);
            memset(buf, 0, 64);
        }

        if (pl->data[1].revents & POLLIN) {
            fgets(buf, sizeof(buf), stdin);
            char *nl = strchr(buf, '\n');
            if (nl) *nl = '\0';
            tup(1);
            tclr();

            info("Got your input from STDIN: %.*s", 64, buf);
            memset(buf, 0, 64);
        }
    }

    return EXIT_SUCCESS;
}

ssize_t readline(int cfd, char *buf, size_t n)
{
    size_t totalread = 0;

    char *line = malloc(128);
    assert(line != NULL, "failed to malloc for readline");

    char ch[1];
    ssize_t numread = 0;
    while (totalread < n - 1) {
        numread = read(cfd, ch, 1);
        if (numread == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        else if (numread == 0) {
            return totalread;
        }
        else {
            *buf++ = *ch;
            totalread++;
            if (*ch == '\n') {
                break;
            }
        }
    }
    *buf = '\0';

    return totalread;
}

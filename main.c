#include "check.h"
#include "errfunc.h"
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
#include <sys/types.h>
#include <unistd.h>

const char *ACKMESSAGE = "Received your message\n";

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
    sa.sin_addr.s_addr = ntohl(INADDR_ANY);

    int result = bind(sfd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
    assert(result != -1, "failed to bind socket to an address");

    char addrstr[INET_ADDRSTRLEN] = {0};
    assert(inet_ntop(AF_INET, (void *)&sa.sin_addr, addrstr, INET_ADDRSTRLEN) != NULL, "failed to convert the address to string");

    info("Serving the server on: %s:%d", addrstr, port);

    listen(sfd, 10);
    struct plist *pl = pl_init(32);
    assert(pl != NULL, "failed to initialize poll list");

    struct pollfd pfd = {.fd = sfd, .events = POLLIN};
    pl_append(pl, pfd);
    pfd = (struct pollfd){.fd = STDIN_FILENO, .events = POLLIN};
    pl_append(pl, pfd);

    int numread = 0;
    char buf[1024] = {0};
    while (1) {
        int ready = poll(pl->data, pl->len, -1);
        if (ready == -1) {
            error("failed on poll syscall");
            continue;
        }

        if (pl->data[0].revents & POLLIN) {
            // ----------- Handle Client Messages ----------------

            int cfd = accept(sfd, NULL, NULL);
            if (cfd == -1) {
                error("error on accept syscall");
            }

            struct pollfd pfd = {.fd = cfd, .events = POLLIN | POLLHUP};
            pl = pl_append(pl, pfd);
        }

        if (pl->data[1].revents & POLLIN) {
            numread = read(STDIN_FILENO, buf, sizeof(buf) - 1);
            if (numread == -1)
                err_exit("read from stdin");
            char *nl = strchr(buf, '\n');
            if (nl) *nl = '\0';
            tup(1);
            tclr();

            info("Got your input from STDIN: %.*s", 1024, buf);
        }

        for (size_t i = pl->len - 1; i >= 2; i--) {
            var("%ld", i);

            struct pollfd pfd = pl->data[i];
            if (pfd.revents & POLLIN) {
                ssize_t n = recv(pfd.fd, buf, sizeof(buf), 0);
                var("%ld", n);

                if (n == 0) {
                    info("closing connection for client fd: %d", pfd.fd);
                    close(pfd.fd);
                    if (pl_remove(pl, pfd) == -1) error("pl_remove");
                    continue;
                }
                else if (n == -1) {
                    perror("recv");
                    close(pfd.fd);
                    if (pl_remove(pl, pfd) == -1) error("pl_remove");
                    continue;
                }

                // TODO: copy the buf to another one to log it
                /* char *nl = strchr(buf, '\n'); */
                /* if (nl) *nl = '\0'; */

                info("received from: [%d], message: %.*s", pfd.fd, 1024, buf);

                // --------------- Send message to the sender ---------------

                ssize_t numwrite = send(pfd.fd, ACKMESSAGE, strlen(ACKMESSAGE), MSG_NOSIGNAL);
                var("to sender: %ld", numwrite);
                if (numwrite == -1) {
                    perror("send");
                    exit(EXIT_FAILURE);
                }

                // ----------------- Broadcast Message -------------------
                for (ssize_t j = pl->len - 1; j >= 2; j--) {
                    if (pl->data[j].fd == pfd.fd)
                        continue;

                    ssize_t numwrite = send(pl->data[j].fd, buf, n, MSG_NOSIGNAL);
                    if (numwrite == -1) {
                        perror("send");
                        exit(EXIT_FAILURE);
                    }
                }

                memset(buf, 0, 1024);
            }
            if (pfd.revents & POLLHUP) {
                info("closing connection for client fd: %d", pfd.fd);
                close(pfd.fd);
                if (pl_remove(pl, pfd) == -1) error("pl_remove");
            }
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

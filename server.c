#include "server.h"
#include "check.h"
#include "log.h"
#include "plist.h"
#include "tutil.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const char *ACKMESSAGE = "Received your message\n";

int server_run(const char *host, int port)
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    check(sfd >= 0, "failed to create a socket file");

    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    unsigned char hostaddr[sizeof(struct in_addr)];
    check(inet_pton(AF_INET, host, hostaddr) == 1, "failed to convert host to address");

    struct sockaddr_in sa;
    sa.sin_port = ntohs(port);
    sa.sin_family = AF_INET;
    sa.sin_addr = *(struct in_addr *)&hostaddr;

    int result = bind(sfd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
    check(result != -1, "failed to bind socket to an address");

    char addrstr[INET_ADDRSTRLEN] = {0};
    check(inet_ntop(AF_INET, (void *)&sa.sin_addr, addrstr, INET_ADDRSTRLEN) != NULL, "failed to convert the address to string");

    info("Serving the server on: %s:%d", addrstr, port);

    listen(sfd, 10);
    struct plist *pl = pl_init(32);
    check(pl != NULL, "failed to initialize poll list");

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
            up(1);
            clrterminal();

            info("Got your input from STDIN: %.*s", 1024, buf);
        }

        for (size_t i = pl->len - 1; i >= 2; i--) {
            struct pollfd pfd = pl->data[i];
            if (pfd.revents & POLLIN) {
                ssize_t n = recv(pfd.fd, buf, sizeof(buf), 0);

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

                info("received from: [%d], message: %.*s", pfd.fd, 1024, buf);

                // --------------- Send message to the sender ---------------
                ssize_t numwrite = send(pfd.fd, ACKMESSAGE, strlen(ACKMESSAGE), MSG_NOSIGNAL);
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
}

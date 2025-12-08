#define _POSIX_C_SOURCE 200809L

#include "box.h"
#include "check.h"
#include "colors.h"
#include "errfunc.h"
#include "estring.h"
#include "log.h"
#include "modes.h"
#include "tutil.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>

struct termios usertp;

static void handler(int sig)
{
    (void)sig;

    scroll(2);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1)
        err_exit("tcsetattr");

    exit(EXIT_SUCCESS);
}

static void tstp_handler(int sig)
{
    (void)sig;

    scroll(2);

    int saved_errno = errno;

    struct termios tp;
    if (tcgetattr(STDIN_FILENO, &tp) == -1)
        err_exit("tcgetattr");

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1)
        err_exit("tcsetattr");

    if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
        err_exit("signal");

    raise(SIGTSTP);

    sigset_t tstp_mask, prev_mask;
    if (sigprocmask(SIG_UNBLOCK, &tstp_mask, &prev_mask) == -1)
        err_exit("sigprocmask unblock");

    if (sigprocmask(SIG_SETMASK, &prev_mask, NULL) == -1)
        err_exit("sigprocmask setmask");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
        err_exit("sigaction");

    if (tcgetattr(STDIN_FILENO, &usertp) == -1)
        err_exit("tcgetattr");

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1)
        err_exit("tcsetattr");

    errno = saved_errno;
}

int user_input(struct box *usr_box, char ch, int sfd);
int server_message(struct box *srv_box, struct box *usr_box, char *buf, size_t buflen);

int client_run(const char *host, int port)
{

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    check(sfd >= 0, "socket");

    unsigned char hostaddr[sizeof(struct in_addr)];
    check(inet_pton(AF_INET, host, hostaddr) == 1, "failed to convert host to address");

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int result = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    check(result != -1, "connect");

    check(ttySetCbreak(STDIN_FILENO, &usertp) == -1, "ttySetCbreak");

    struct sigaction sa, prev;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;

    check(sigaction(SIGQUIT, NULL, &prev) == -1, "sigaction SIGQUIT");
    if (prev.sa_handler != SIG_IGN)
        check(sigaction(SIGQUIT, &sa, NULL) == -1, "sigaction SIGQUIT");

    check(sigaction(SIGINT, NULL, &prev) == -1, "sigaction SIGINT");
    if (prev.sa_handler != SIG_IGN)
        check(sigaction(SIGINT, &sa, NULL) == -1, "sigaction SIGINT");

    sa.sa_handler = tstp_handler;
    check(sigaction(SIGTSTP, NULL, &prev) == -1, "sigaction SIGTSTP");
    if (prev.sa_handler != SIG_IGN)
        check(sigaction(SIGTSTP, &sa, NULL) == -1, "sigaction SIGTSTP");

    int row, column;
    get_cursor_pos(&row, &column);

    string_t str = str_init();
    struct box *svr_box = box_init(row, column, str, green);

    string_t user_str = str_init();
    struct box *input_box = box_init(row, 1, user_str, blue);

    box_draw_input(input_box);
    move_cursor(input_box->row + 1, input_box->column + input_box->text->len + 1);

    struct pollfd pfds[2] = {0};
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = sfd;
    pfds[1].events = POLLIN | POLLHUP;

    int nread, nserv;
    char ch;
    char servr_buf[1024] = {0};

    while (1) {
        int ready = poll(pfds, 2, -1);
        if (ready == -1) {
            if (errno == EINTR)
                continue;
            error("failed on poll syscall");
            return -1;
        }

        if (pfds[0].revents & POLLIN) {
            nread = read(STDIN_FILENO, &ch, 1);
            if (nread == -1) {
                close(sfd);
                error("read from stdin");
                return -1;
            }
            user_input(input_box, ch, sfd);
        }

        if (pfds[1].revents & POLLIN) {
            nserv = read(sfd, servr_buf, sizeof(servr_buf) - 1);
            if (nserv == 0) {
                close(sfd);
                break;
            }
            if (nserv == -1) {
                close(sfd);
                error("read from socket");
                return -1;
            }
            char *nl = strchr(servr_buf, '\n');
            if (nl) {
                *nl = '\0';
                nserv -= 1;
            }

            server_message(svr_box, input_box, servr_buf, nserv);

            memset(servr_buf, 0, nserv);
        }
        if (pfds[1].revents & POLLHUP) {
            close(sfd);
            break;
        }
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1)
        err_exit("tcsetattr");

    scroll(2);

    return EXIT_SUCCESS;
}

int server_message(struct box *srv_box, struct box *usr_box, char *buf, size_t buflen)
{
    if (srv_box == NULL)
        return EINVAL;

    int row, column;
    get_cursor_pos(&row, &column);

    row = usr_box->row + 2;

    // Scroll if needed more lines
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (w.ws_row - row < 3) {
        move_cursor(w.ws_row, 1);
        scroll(3 - (w.ws_row - row));
        usr_box->row -= 3 - (w.ws_row - row);
    }

    srv_box->row = usr_box->row;
    srv_box->column = 1;
    str_set(srv_box->text, buf, buflen);

    move_cursor(usr_box->row, 1);
    shift(3);

    box_draw(srv_box);

    usr_box->row += 3;
    move_cursor(usr_box->row + 1, column);

    return 0;
}

int user_input(struct box *input_box, char ch, int sfd)
{
    if (input_box == NULL)
        return EINVAL;

    if (ch != '\r') {
        box_addcolumn(input_box, ch);
        return 0;
    }

    box_done(input_box);

    if (input_box->text->len > 0 &&
        send(sfd, input_box->text->data, input_box->text->len, MSG_DONTWAIT) == -1)
        if (!(errno == EAGAIN || errno == EWOULDBLOCK))
            error("write to socket");

    int row = input_box->row + 2;

    // Scroll if needed more lines
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (w.ws_row - row < 3) {
        move_cursor(w.ws_row, 1);
        scroll(3 - (w.ws_row - row));
        row = w.ws_row - 2;
    }
    else {
        row = input_box->row + 3;
    }

    str_clear(input_box->text);
    input_box->row = row;
    input_box->column = 1;
    box_draw_input(input_box);
    move_cursor(input_box->row + 1, input_box->column + input_box->text->len + 1);

    return 0;
}

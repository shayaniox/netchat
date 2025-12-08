#include "client.h"
#include "errfunc.h"
#include "server.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8080

struct config {
    char *host;
    int port;
    int is_server;
};

void print_usage(char *prog_name)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s client -h <host>[default: 127.0.0.1] -p <port>[default: 8080]\n", prog_name);
    fprintf(stderr, "  %s server -h <host>[default: 127.0.0.1] -p <port>[default: 8080]\n", prog_name);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  -h <host>    Host/IP address\n");
    fprintf(stderr, "  -p <port>    Port number\n");
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  %s client -h 127.0.0.1 -p 8080\n", prog_name);
    fprintf(stderr, "  %s server -h 0.0.0.0 -p 8080\n", prog_name);
}

int parse_args(int argc, char *argv[], struct config *cfg)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return -1;
    }

    if (strcmp(argv[1], "server") == 0) {
        cfg->is_server = 1;
    }
    else if (strcmp(argv[1], "client") == 0) {
        cfg->is_server = 0;
    }
    else {
        fprintf(stderr, "Error: Invalid mode '%s'. Must be 'client' or 'server'\n", argv[1]);
        print_usage(argv[0]);
        return -1;
    }

    int opt;
    optind = 2;
    for (; (opt = getopt(argc, argv, "h:p:")) != -1;) {
        switch (opt) {
        case 'h':
            cfg->host = optarg;
            break;
        case 'p':
            cfg->port = atoi(optarg);
            if (cfg->port <= 0)
                err_exit("Port option must be numeric");
            break;
        default:
            print_usage(argv[0]);
            return -1;
        }
    }

    if (cfg->host == NULL) {
        cfg->host = DEFAULT_HOST;
    }

    if (cfg->port == 0) {
        cfg->port = DEFAULT_PORT;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    struct config *cfg = malloc(sizeof(struct config));
    if (parse_args(argc, argv, cfg) == -1)
        exit(EXIT_FAILURE);

    if (cfg->is_server) {
        if (server_run(cfg->host, cfg->port) == -1)
            err_exit("server_run");
    }
    else {
        if (client_run(cfg->host, cfg->port) == -1)
            err_exit("client_run");
    }

    free(cfg);

    return EXIT_SUCCESS;
}

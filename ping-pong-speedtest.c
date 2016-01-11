#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <error.h>
#include <errno.h>

unsigned short port  = 80;
const char*    ip    = "127.0.0.1";

static void
tick(void)
{
    static unsigned count = 0;
    static struct timeval past;
    struct timeval now;
    count++;

    gettimeofday(&now, NULL);

    if (past.tv_sec != now.tv_sec) {
        past = now;
        printf("%u\n", count);
        count = 0;
    }
}

static void
set_socket_options(int sock)
{
    int ret;
    int flags = 1;

    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
    if (ret == -1) {
        printf("setsocketopt() failed to set reuse address flag: (errno=%i) %s\n", errno, strerror(errno));
        exit(1);
    }

    struct linger linger_option;
    linger_option.l_onoff  = 0;
    linger_option.l_linger = 0;
    ret = setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger_option, sizeof(linger_option));
    if (ret == -1) {
        printf("setsocketopt() failed to set linger options: (errno=%i) %s\n", errno, strerror(errno));
        exit(1);
    }

    ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags));
    if (ret == -1) {
        printf("setsocketopt() failed to set TCP no delay flag: (errno=%i) %s\n", errno, strerror(errno));
        exit(0);
    }
}

int
main(int argc, char * argv[])
{
    char  buf[1500];
    const char* message;

    if (argc != 2) {
        printf("./ping-ping-speedtest <'server'|'client'>\n");
        exit(1);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("socket() failed: (errno=%i) %s\n", errno, strerror(errno));
        exit(1);
    }

    set_socket_options(sock);

    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (argv[1][0] == 's') { // Server
        printf("Server mode\n");
        message = "Pong";
        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            printf("bind(): failed (errno=%i) %s, addr=%s:%u\n", errno, strerror(errno), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            return 1;
        }
        printf("Bound\n");
        if (listen(sock, 1) != 0) {
            printf("listen() failed: (errno=%i) %s\n", errno, strerror(errno));
            exit(1);
        }
        printf("Listening\n");
        struct sockaddr_in new_addr;
        socklen_t new_addr_size = sizeof(new_addr);
        int new_sock = accept(sock, (struct sockaddr *)&new_addr, &new_addr_size);
        if (new_sock == -1) {
            printf("accept() failed: (errno=%i) %s\n", errno, strerror(errno));
            exit(1);
        }
        printf("Accepted\n");

        while (1) {
            int length = recv(new_sock, buf, sizeof(buf), 0);
            if (length != (int)strlen(message)) {
                printf("recv() failed!\n");
                exit(1);
            } 
            if (send(new_sock, message, strlen(message), MSG_NOSIGNAL) != (int)strlen(message)) {
                printf("send(): Error writing to socket: (errno=%i) %s\n", errno, strerror(errno));
            }
            tick();
        }
    } else { // Client
        printf("Client mode\n");
        message = "Ping";
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            printf("connect(): failed (errno=%i) %s, addr=%s:%u\n", errno, strerror(errno), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            return 1;
        }
        printf("Connected\n");

        // The client starts the ping-pong
        while (1) {
            if (send(sock, message, strlen(message), MSG_NOSIGNAL) != (int)strlen(message)) {
                printf("send(): Error writing to socket: (errno=%i) %s\n", errno, strerror(errno));
            }
            int length = recv(sock, buf, sizeof(buf), 0);
            if (length != (int)strlen(message)) {
                printf("recv() failed!\n");
                exit(1);
            } 
            tick();
        }
    }

    return 0;
}

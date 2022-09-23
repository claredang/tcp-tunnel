#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>-
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() */
#define DAYTIME_PORT 3333

struct message {
    int addrlen, timelen, msglen;
    char addr[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

int main(int argc, char **argv)
{
    int     listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr; 
    socklen_t len;
    char    buff[MAXLINE];
    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    char* p;
    int argv_to_int = strtol(argv[1], &p, 10);
    servaddr.sin_port = htons(argv_to_int);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);
    

    for ( ; ; ) {
        // ===== 1. Listen from client
        len = sizeof(cliaddr);

        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);

        // Thread and concurrent 
        pid_t pid;
        if( (pid = fork()) == 0) {
            close(listenfd);
            close(connfd);
            exit(0);
        }

        const char* client_ip = inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff));
        int client_port = ntohs(cliaddr.sin_port);
        // printf("Connection from %s, port %d\n", client_ip, client_port);

        // getnameinfo: server name -> IP address
        struct sockaddr_in sa;
        char node[NI_MAXHOST];

        memset(&sa, 0, sizeof sa);
        sa.sin_family = AF_INET;
        
        inet_pton(AF_INET, client_ip, &sa.sin_addr);

        int res2 = getnameinfo((struct sockaddr*)&sa, sizeof(sa),
                            node, sizeof(node),
                            NULL, 0, NI_NAMEREQD);
        
        if (res2) {
            printf("error: %d\n", res2);
            printf("%s\n", gai_strerror(res2));
        }
        else {
            printf("Server name: %s\n", node);
        }

        // getaddrinfo: IP address -> server name
        struct addrinfo* res = NULL;
        struct addrinfo hints;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        getaddrinfo(client_ip, NULL, &hints, &res);

        printf("IP address: %s\n", client_ip);
        
        // ===== 2. Construct struct message to send
        struct message msg;
        ticks = time(NULL);
        snprintf(msg.currtime, strlen(ctime(&ticks)), "%.24s\r\n", ctime(&ticks));
       
        // Run who command
        FILE * f = popen( "who|sort", "r" );
        if ( f == 0 ) {
            fprintf( stderr, "Could not execute\n" );
            return 1;
        }
        const int BUFSIZE = 1000;
        char payload[ BUFSIZE ];
        while( fgets( payload, BUFSIZE,  f ) ) {}
        pclose(f);

        snprintf(msg.payload, strlen(payload), payload);
        write(connfd, &msg, sizeof(msg));

        close(connfd);
    }
}


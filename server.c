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
        printf("Connection from %s, port %d\n", client_ip, client_port);

        // ===== 2. Construct struct message to send
        struct message msg;
        ticks = time(NULL);
        snprintf(msg.currtime, sizeof(msg.currtime), "%.24s\r\n", ctime(&ticks));
        // printf("Sending time: %d\n", msg.currtime);

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

        snprintf(msg.payload, sizeof(msg.payload), payload);
        write(connfd, &msg, sizeof(msg));
        // printf("Sending who command: %s\n", msg.payload);


        close(connfd);
    }
}


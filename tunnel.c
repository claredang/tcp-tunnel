// #include <netinet/in.h>
// #include <time.h>
// #include <strings.h>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/types.h>
// #include <sys/socket.h>


#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
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
    struct sockaddr_in servaddr, cliaddr, dest_addr;
    socklen_t len;
    char    buff[MAXLINE];
    time_t ticks;
    int sockfd, n;
    char    recvline[MAXLINE + 1];

    // Establish tunnel to receive message from client
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    char* p;
    int argv_to_int = strtol(argv[1], &p, 10);
    servaddr.sin_port = htons(argv_to_int);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    // Connect to server
    len = sizeof(cliaddr);

    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
    printf("Connection from %s, port %d\n",inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));


    // ========= 3. Establish connection to server ==== ///
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        exit(1);
    }

    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    // Change argv to int 
    // char* p;
    // int argv_to_int = strtol(argv[2], &p, 10);
    // servaddr.sin_addr.s_addr = inet_addr[]
    inet_aton("10.0.0.83", &dest_addr.sin_addr.s_addr); // server_ip
    dest_addr.sin_port = htons(4444); // server_port

    // if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
    //     printf("inet_pton error for %s\n", argv[1]);
    //     exit(1);
    // }

    if (connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) < 0) {
        printf("connect error\n");
        exit(1);
    }


    // Read message from server
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;        /* null terminate */
        if (fputs(recvline, stdout) == EOF) {
            printf("fputs error\n");
            exit(1);
        }
        printf("Tunnel: Read from server successfully\n");
        // Send message to client
        struct message msg;
        ticks = time(NULL);
        snprintf(msg.currtime, sizeof(msg.currtime), "%.24s\r\n", ctime(&ticks));
        msg.timelen = sizeof(msg.currtime);
        write(connfd, msg.currtime, msg.timelen);
        printf("Sending response: %s", msg.currtime);

        exit(1);
    }
    if (n < 0) {
        printf("read error\n");
        exit(1);
    }
   
    close(connfd);
}


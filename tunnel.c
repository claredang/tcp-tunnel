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
    int sockfd, n;

    // 1. ========= Establish tunnel to receive message from client
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    char* p;
    int argv_to_int = strtol(argv[1], &p, 10);
    servaddr.sin_port = htons(argv_to_int);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    len = sizeof(cliaddr);

    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
    printf("Receive request from client %s port %d ",inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));

    // Receive client's message:
    struct message msg_in;
    if ((n = recv(connfd, &msg_in, sizeof(msg_in), MSG_WAITALL)) == -1) { 
        perror("recv");
        exit(1); 
    }
    printf("destined to server %s port %s\n", msg_in.addr, msg_in.payload);

    // ========= 2. Establish connection to server ==== ///
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        exit(1);
    }

    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    // Change argv to int 
    char* p2;
    int argv_to_int2 = strtol(msg_in.payload, &p2, 10);
    inet_aton(msg_in.addr, &dest_addr.sin_addr.s_addr); // server_ip
    dest_addr.sin_port = htons(argv_to_int2); // server_port

    // if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
    //     printf("inet_pton error for %s\n", argv[1]);
    //     exit(1);
    // }

    if (connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) < 0) {
        printf("connect error\n");
        exit(1);
    }

    // Read message from server
    struct message server_msg;
    int numbytes2;
    if ((numbytes2 = recv(sockfd, &server_msg, sizeof(server_msg), MSG_WAITALL)) == -1) { 
        perror("recv");
        exit(1); 
    }
    // printf("tunnel: receive from server %d'\n", server_msg.timelen);

    // Send message to client
    write(connfd, &server_msg, sizeof(server_msg));


    if (n < 0) {
        printf("read error\n");
        exit(1);
    }
   
    close(connfd);
}


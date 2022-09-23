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

#define MAXLINE     4096    /* max text line length */
#define DAYTIME_PORT 3333

struct message {
    int addrlen, timelen, msglen;
    char addr[MAXLINE];
    char currtime[MAXLINE];
    char payload[MAXLINE];
};

int main(int argc, char **argv)
{
    int     sockfd, n;
    char    recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if (argc != 3 && argc != 5) {
        printf("usage: client <server_ip> <server_port>\n");
        printf("usage: client <tunnel_ip> <tunnel_port> <server_ip> <server_port>\n");
        exit(1);
    }
    
    if (argc == 3) {
        // =====  1. getnameinfo: server name -> IP address
        struct sockaddr_in sa;
        char node[NI_MAXHOST];

        memset(&sa, 0, sizeof sa);
        sa.sin_family = AF_INET;
        
        inet_pton(AF_INET, argv[1], &sa.sin_addr);

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

        // ===== 2. getaddrinfo: IP address -> server name
        struct addrinfo* res = NULL;
        struct addrinfo hints;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        getaddrinfo(argv[1], NULL, &hints, &res);

        printf("IP address: ");
        struct addrinfo* i;
        for(i=res; i!=NULL; i=i->ai_next)
        {
            char str[INET6_ADDRSTRLEN];
            if (i->ai_addr->sa_family == AF_INET) {
                struct sockaddr_in *p = (struct sockaddr_in *)i->ai_addr;
                printf("%s\n", inet_ntop(AF_INET, &p->sin_addr, str, sizeof(str)));
            } else if (i->ai_addr->sa_family == AF_INET6) {
                struct sockaddr_in6 *p = (struct sockaddr_in6 *)i->ai_addr;
                printf("%s\n", inet_ntop(AF_INET6, &p->sin6_addr, str, sizeof(str)));
            }
        }


        // ========= 3. Establish socket and print server message 
        if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("socket error\n");
            exit(1);
        }

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        // Change argv to int 
        char* p;
        int argv_to_int = strtol(argv[2], &p, 10);
        servaddr.sin_port = htons(argv_to_int);

        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
            printf("inet_pton error for %s\n", argv[1]);
            exit(1);
        }

        if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            printf("connect error\n");
            exit(1);
        }

        char server_message[2000], client_message[2000];
        // Clean buffers:
        memset(client_message,'\0',sizeof(client_message));
        
        // Get input from the user:
        printf("Enter message: ");
        gets(client_message);
        

        if(send(sockfd, client_message, strlen(client_message), 0) < 0){
            printf("Unable to send message\n");
            exit(1);
        }

        while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
            recvline[n] = 0;        /* null terminate */
            if (fputs(recvline, stdout) == EOF) {
                printf("fputs error\n");
                exit(1);
            }
            exit(1);
        }
        if (n < 0) {
            printf("read error\n");
            exit(1);
        }
        
        exit(0);
    }

    else if (argc == 5) {
        // =====  1. getnameinfo: server name -> IP address
        struct sockaddr_in sa;
        char node[NI_MAXHOST];

        memset(&sa, 0, sizeof sa);
        sa.sin_family = AF_INET;
        
        inet_pton(AF_INET, argv[1], &sa.sin_addr);

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

        // ===== 2. getaddrinfo: IP address -> server name
        struct addrinfo* res = NULL;
        struct addrinfo hints;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        getaddrinfo(argv[1], NULL, &hints, &res);

        printf("IP address: ");
        struct addrinfo* i;
        for(i=res; i!=NULL; i=i->ai_next)
        {
            char str[INET6_ADDRSTRLEN];
            if (i->ai_addr->sa_family == AF_INET) {
                struct sockaddr_in *p = (struct sockaddr_in *)i->ai_addr;
                printf("%s\n", inet_ntop(AF_INET, &p->sin_addr, str, sizeof(str)));
            } else if (i->ai_addr->sa_family == AF_INET6) {
                struct sockaddr_in6 *p = (struct sockaddr_in6 *)i->ai_addr;
                printf("%s\n", inet_ntop(AF_INET6, &p->sin6_addr, str, sizeof(str)));
            }
        }


        // ========= 3. Establish socket and print server message 
        if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("socket error\n");
            exit(1);
        }

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        // Change argv to int 
        char* p;
        int argv_to_int = strtol(argv[2], &p, 10);
        servaddr.sin_port = htons(argv_to_int);

        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
            printf("inet_pton error for %s\n", argv[1]);
            exit(1);
        }

        if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            printf("connect error\n");
            exit(1);
        }

        char server_message[2000], client_message[2000];
        // Clean buffers:
        memset(client_message,'\0',sizeof(client_message));
        
        // // Get input from the user:
        // printf("Enter message: ");
        // gets(client_message);
        

        // if(send(sockfd, client_message, strlen(client_message), 0) < 0){
        //     printf("Unable to send message\n");
        //     exit(1);
        // }

        while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
            recvline[n] = 0;        /* null terminate */
            if (fputs(recvline, stdout) == EOF) {
                printf("fputs error\n");
                exit(1);
            }
            exit(1);
        }
        if (n < 0) {
            printf("read error\n");
            exit(1);
        }
        
        exit(0);

    }
    exit(0);
}

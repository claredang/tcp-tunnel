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
    
    // CASE 1: CLIENT -> SERVER
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


        // ===== 3. Establish socket and print server message 
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

        // Receive message from client
        struct message server_msg;
        if ((n = recv(sockfd, &server_msg, sizeof(server_msg), MSG_WAITALL)) == -1) { 
            perror("recv");
            exit(1); 
        }
        printf("Time: %s\n", server_msg.currtime);
        printf("Who:  %s\n", server_msg.payload);
    
        exit(0);
    }
    


    // CASE 2: CLIENT -> TUNNEL -> SERVER
    else if (argc == 5) {
        // client <tunnel_ip> <tunnel_port> <server_ip> <server_port>
        //   0        1             2           3           4
        
        // =====  1. getnameinfo: server name -> IP address
        struct sockaddr_in sa;
        char node[NI_MAXHOST];

        memset(&sa, 0, sizeof sa);
        sa.sin_family = AF_INET;
        
        inet_pton(AF_INET, argv[3], &sa.sin_addr);

        int res1 = getnameinfo((struct sockaddr*)&sa, sizeof(sa),
                            node, sizeof(node),
                            NULL, 0, NI_NAMEREQD);
        
        if (res1) {
            printf("error: %d\n", res1);
            printf("%s\n", gai_strerror(res1));
        }
        else {
            printf("Server name: %s\n", node);
        }

        // ===== 2. getaddrinfo: IP address -> server name
        struct addrinfo* res = NULL;
        struct addrinfo hints;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        getaddrinfo(argv[3], NULL, &hints, &res);

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


        // ===== 3. Establish socket and print server message 
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

        // Send message to tunnel
        struct message msg;
        strcpy(msg.addr, argv[3]); // Send server_ip
        strcpy(msg.payload, argv[4]); // Send server_port
        // printf("Message send: %s\n", msg.addr);

        if(send(sockfd, &msg, sizeof(msg), 0) < 0){
            printf("Unable to send message\n");
            exit(1);
        }

        // Receive message from tunnel
        struct message server_msg;
        if ((n = recv(sockfd, &server_msg, sizeof(server_msg), MSG_WAITALL)) == -1) { 
            perror("recv");
            exit(1); 
        }
        printf("Time: %s\n", server_msg.currtime);
        printf("Who:  %s\n", server_msg.payload);


        if (n < 0) {
            printf("read error\n");
            exit(1);
        }
        


        // ===== Get Tunnel info
        // getnameinfo: IP address -> name
        inet_pton(AF_INET, argv[1], &sa.sin_addr);

        int res2 = getnameinfo((struct sockaddr*)&sa, sizeof(sa),
                            node, sizeof(node),
                            NULL, 0, NI_NAMEREQD);
        
        if (res2) {
            printf("error: %d\n", res2);
            printf("%s\n", gai_strerror(res2));
        }
        else {
            printf("\n\nVia tunnel: %s\n", node);
        }

        // getaddrinfo: name -> IP 
        printf("IP address: ");
        getaddrinfo(argv[1], NULL, &hints, &res);
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

        printf("Port number: %s\n", argv[4]);

        exit(0);

    }
    exit(0);
}

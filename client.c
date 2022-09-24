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


// ssh pdang@csil-cpu2.csil.sfu.ca -p 24 : 206.12.16.199
// ssh pdang@csil-cpu3.csil.sfu.ca -p 24 : 206.12.16.201
// ssh pdang@csil-cpu3.csil.sfu.ca -p 24 : 142.58.22.188
// ssh pdang@csil-cpu4.csil.sfu.ca -p 24 : 142.58.22.188
// ssh pdang@csil-cpu5.csil.sfu.ca -p 24 : 127.0.1.1
// ssh pdang@csil-cpu6.csil.sfu.ca -p 24 : 127.0.1.1
// ./client 206.12.16.201 3333 206.12.16.199 4444  / Tunnel 3 Server 2
// ./client csil-cpu3.csil.sfu.ca 3333 csil-cpu2.csil.sfu.ca 4444  / Tunnel 3 Server 2

// Server name: csil-cpu2.cs.surrey.sfu.ca

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
        struct addrinfo hints, *servinfo, *p2;
        int rv;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
        hints.ai_socktype = SOCK_STREAM;


        // ===== 2. getaddrinfo: IP address -> server name
        struct addrinfo* res = NULL;
        // struct addrinfo hints;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        getaddrinfo(argv[1], NULL, &hints, &res);

        printf("IP Address: ");
        struct addrinfo* i;
        char name_ip_convert[100];
        for(i=res; i!=NULL; i=i->ai_next)
        {
            char str[INET6_ADDRSTRLEN];
            if (i->ai_addr->sa_family == AF_INET) {
                struct sockaddr_in *p = (struct sockaddr_in *)i->ai_addr;
                strcpy(name_ip_convert, inet_ntop(AF_INET, &p->sin_addr, str, sizeof(str)));
                printf("%s\n", name_ip_convert);
                break;
            } else if (i->ai_addr->sa_family == AF_INET6) {
                struct sockaddr_in6 *p = (struct sockaddr_in6 *)i->ai_addr;
                printf("%s\n", inet_ntop(AF_INET6, &p->sin6_addr, str, sizeof(str)));
                break;
            }
        }

        // =====  2.1. getnameinfo: server name -> IP address
        struct sockaddr_in sa;
        char node[NI_MAXHOST];

        memset(&sa, 0, sizeof sa);
        sa.sin_family = AF_INET;
        
        inet_pton(AF_INET, name_ip_convert, &sa.sin_addr);

        int res2 = getnameinfo((struct sockaddr*)&sa, sizeof(sa),
                            node, sizeof(node),
                            NULL, 0, NI_NAMEREQD);
        
        if (res2) {
            printf("error: %d\n", res2);
            printf("%s\n", gai_strerror(res2));
        }
        else {
            printf("Server Name: %s\n", node);
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

        // if argv = csil -> pton need to use name_ip_convert
        // if argv = ip -> use argv[1]

        // printf("name_ip_convert: %s \n", name_ip_convert);

        if (inet_pton(AF_INET, name_ip_convert, &servaddr.sin_addr) <= 0) {
            printf("inet_pton error for: %s\n", argv[1]);
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
        

        struct addrinfo hints, *servinfo, *p2;
        int rv;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
        hints.ai_socktype = SOCK_STREAM;


        // ===== 2. getaddrinfo: IP address -> server name
        struct addrinfo* res = NULL;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        getaddrinfo(argv[3], NULL, &hints, &res);

        printf("IP Address: ");
        struct addrinfo* i;
        char name_ip_convert_server[100];

        for(i=res; i!=NULL; i=i->ai_next)
        {
            char str[INET6_ADDRSTRLEN];
            if (i->ai_addr->sa_family == AF_INET) {
                struct sockaddr_in *p = (struct sockaddr_in *)i->ai_addr;
                strcpy(name_ip_convert_server, inet_ntop(AF_INET, &p->sin_addr, str, sizeof(str)));
                printf("%s\n", name_ip_convert_server);
                break;
            } else if (i->ai_addr->sa_family == AF_INET6) {
                struct sockaddr_in6 *p = (struct sockaddr_in6 *)i->ai_addr;
                printf("%s\n", inet_ntop(AF_INET6, &p->sin6_addr, str, sizeof(str)));
                break;
            }
        }
        // printf("name_ip_convert_server: %s \n", name_ip_convert_server);


        // Convert for tunnel
        struct addrinfo* res2 = NULL;
        
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        getaddrinfo(argv[1], NULL, &hints, &res2);

        // printf("IP Address: ");
        struct addrinfo* i2;
        char name_ip_convert_tunnel[100];

        for(i2=res2; i2!=NULL; i2=i2->ai_next)
        {
            char str[INET6_ADDRSTRLEN];
            if (i2->ai_addr->sa_family == AF_INET) {
                struct sockaddr_in *p = (struct sockaddr_in *)i2->ai_addr;
                strcpy(name_ip_convert_tunnel, inet_ntop(AF_INET, &p->sin_addr, str, sizeof(str)));
                // printf("%s\n", name_ip_convert_tunnel);
                break;
            } else if (i2->ai_addr->sa_family == AF_INET6) {
                struct sockaddr_in6 *p = (struct sockaddr_in6 *)i2->ai_addr;
                printf("%s\n", inet_ntop(AF_INET6, &p->sin6_addr, str, sizeof(str)));
                break;
            }
        }
        // printf("name_ip_convert_tunnel: %s \n", name_ip_convert_tunnel);


        // =====  2.1. getnameinfo: server name -> IP address
        struct sockaddr_in sa;
        char node[NI_MAXHOST];

        memset(&sa, 0, sizeof sa);
        sa.sin_family = AF_INET;
        
        inet_pton(AF_INET, name_ip_convert_server, &sa.sin_addr);

        int res4 = getnameinfo((struct sockaddr*)&sa, sizeof(sa),
                            node, sizeof(node),
                            NULL, 0, NI_NAMEREQD);
        
        if (res4) {
            printf("error: %d\n", res4);
            printf("%s\n", gai_strerror(res4));
        }
        else {
            printf("Server Name: %s\n", node);
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

        if (inet_pton(AF_INET, name_ip_convert_tunnel, &servaddr.sin_addr) <= 0) {
            printf("inet_pton error for %s\n", argv[1]);
            exit(1);
        }

        if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            printf("connect error\n");
            exit(1);
        }

        // Send message to tunnel
        struct message msg;
        // strcpy(msg.addr, argv[3]); // Send server_ip
        strcpy(msg.addr, name_ip_convert_server);// Send server_ip
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
        inet_pton(AF_INET, name_ip_convert_tunnel, &sa.sin_addr);

        int res3 = getnameinfo((struct sockaddr*)&sa, sizeof(sa),
                            node, sizeof(node),
                            NULL, 0, NI_NAMEREQD);
        
        if (res3) {
            printf("error: %d\n", res3);
            printf("%s\n", gai_strerror(res3));
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
                break;
            } else if (i->ai_addr->sa_family == AF_INET6) {
                struct sockaddr_in6 *p = (struct sockaddr_in6 *)i->ai_addr;
                printf("%s\n", inet_ntop(AF_INET6, &p->sin6_addr, str, sizeof(str)));
                break;
            }
        }

        printf("Port number: %s\n", argv[4]);



        
        exit(0);

    }
    exit(0);
}

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

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_port = htons(DAYTIME_PORT); /* daytime server */
    char* p;
    int argv_to_int = strtol(argv[1], &p, 10);
    servaddr.sin_port = htons(argv_to_int);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);
    
    char buff2[1000];

    for ( ; ; ) {
        len = sizeof(cliaddr);

        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
        printf("Connection from %s, port %d\n",inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));

        // Receive client's message:
        bzero(buff2, 1000);
        char server_message[2000], client_message[2000];
        read(connfd, buff2, sizeof(buff2));
        printf("Message received: %s\n: ", buff2);
        // // Clean buffers:
        // memset(server_message, '\0', sizeof(server_message));
        // memset(client_message, '\0', sizeof(client_message));

        // if (recv(listenfd, client_message, sizeof(client_message), 0) < 0){
        // // if (read(listenfd, client_message, sizeof(client_message)) < 0){
        //     printf("Couldn't receive\n");
        //     return -1;
        // }
        // printf("Msg from client: %s\n", client_message);
    

        // // Sending time 
        // ticks = time(NULL);
        // snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        // write(connfd, buff, strlen(buff));
        // printf("Sending response: %s", buff);

        int sockfd;
         // 3. Establish socket and print server message 
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        exit(1);
    }

    bzero(&dest_addr, sizeof(dest_addr));
    servaddr.sin_family = AF_INET;
    // servaddr.sin_port = htons(DAYTIME_PORT);  /* daytime server */
    // Change argv to int 
    char* p;
    int argv_to_int = strtol(argv[2], &p, 10);
    servaddr.sin_port = htons(argv_to_int);

        if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            printf("connect error\n");
            exit(1);
        }

        // // Sending who command 
        // struct message msg;
        // // Run who command and send it backs to client
        // FILE * f = popen( "who|sort", "r" );
        // if ( f == 0 ) {
        //     fprintf( stderr, "Could not execute\n" );
        //     return 1;
        // }
        // const int BUFSIZE = 1000;
        // char buf[ BUFSIZE ];
        // while( fgets( buf, BUFSIZE,  f ) ) {
        //     fprintf( stdout, "%s", buf  );
        // }
        // pclose(f);
        // // snprintf(msg.payload, sizeof(msg.payload), buf);
        // write(connfd, buf, strlen(buf));
        // printf("Sending who command: %s", buf);


        close(connfd);
    }
}


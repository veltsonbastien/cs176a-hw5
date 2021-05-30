// Code from TCP Example Code given on Gauchospace
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

void error(const char * msg) {
  perror(msg);
  exit(0);
}

int main(int argc, char * argv[]) {

  //First thing, Client asks if user wants to join game: 
  int yon;
  printf("Ready to start game? (y/n): ");
  yon = getchar( );
  
  //only start the connection and all that if they say yes: 
  if(yon == 'y'){
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent * server;

    char buffer[256];
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char * ) & serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char * ) server -> h_addr,
        (char * ) & serv_addr.sin_addr.s_addr,
        server -> h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

   bzero(buffer, 256);
    //AT THIS POINT, EVERYTHING FOR CONNECTION HAS BEEN ESTABLISHED, send over the 0 message to the server  
    // fgets(buffer, 255, stdin);
    n = write(sockfd, "0", 1);
    // if (n < 0)
    //     error("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");
    while (strlen(buffer) > 0) { 
        //Continue to print messages from server while we are still reading from it:
        printf("%s\n", buffer);
        if(strcmp(buffer, "server-overloaded") == 0){ //If the message is an overload error, end the connection
         close(sockfd);
         return 0 ;
        }
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0) error("ERROR reading from socket");
    }
    //Close the socket:
    close(sockfd);
  } //end of if yon is y
  return 0;
}
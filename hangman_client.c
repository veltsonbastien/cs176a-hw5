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

//Delay code inspired from: https://www.geeksforgeeks.org/time-delay-c/
void delay(int number_of_seconds) {
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
  
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
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
    buffer[0] = '0';
    buffer[1] = ' '; 
    buffer[2] = '\0';
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) error("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");
    //While there is still something to read on the buffer:
    while (strlen(buffer) > 0) { 
        if(strcmp(buffer, "server-overloaded") == 0){ //If the message is an overload error, end the connection
         close(sockfd);
         return 0;
        }
        //If runtime continues, now check the flag:
        if(buffer[0] == '0'){
            //in this case, we know we are still running game logic 
            memmove(buffer, buffer+1, strlen(buffer)); //inspired from: https://stackoverflow.com/questions/4295754/how-to-remove-first-character-from-c-string
            printf("%s\n", buffer); //print out the buffer 
            bzero(buffer, 256); // zero out the buffer 
            printf("Letter to guess: "); 
            char guess[255]; 
            scanf("%s", guess); //get the guess from user input 
            if(strlen(guess) == 0) buffer[0] = '0'; 
            if(strlen(guess) == 1) buffer[0] = '1'; 
            if(strlen(guess) >  1) buffer[0] = '2'; //since it doesn't matter really how long it is, server will handle that it was too large
            strcat(buffer, guess);
            //printf("Buffer is %s and its' length is %ld", buffer, strlen(buffer));
            n = write(sockfd, buffer, strlen(buffer)); //send it back to server
        }
        if(buffer[0] == '8'){ 
            //in this case we know that we have won
            memmove(buffer, buffer+1, strlen(buffer)); //inspired from: https://stackoverflow.com/questions/4295754/how-to-remove-first-character-from-c-string
            printf("%s\n", buffer); //print out the buffer 
            close(sockfd); //end the connection
            return 0; //end the program
        }    
        //keep on reading from buffer;
        n = read(sockfd, buffer, 255);
        if (n < 0) error("ERROR reading from socket");
    }
    //Close the socket:
    close(sockfd);
  } //end of if yon is y
  return 0;
}
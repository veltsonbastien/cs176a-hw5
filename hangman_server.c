#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

void error(const char * msg) {
  perror(msg);
  exit(1);
}

//Delay code inspired from: https://www.geeksforgeeks.org/time-delay-c/
void delay(int number_of_seconds) {
  // Converting time into milli_seconds
  int milli_seconds = number_of_seconds;
  // Storing start time
  clock_t start_time = clock();
  // looping till required time is not achieved
  while (clock() < start_time + milli_seconds)
  ;
}

char * sumOfDigits(char * bufferCopy) {
  int total = 0;
  /* Do a linear add of the numbers that come in only if they are digits */
  for (int i = 0; i < strlen(bufferCopy); i++) {
    char currentChar = bufferCopy[i];
    if ((currentChar >= '0' && currentChar <= '9')) {
      total += (currentChar) - '0';
    } else { 
      //This will return a letter to signify letters were found:
      char * result = malloc(2);
      result[0] = 'a';
      result[1] = '\0';
      return result;
    }
  }
  //Converting int to char*, with answer inspired from: 
  //https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
  if (strlen(bufferCopy) > 0) { 
    //Only output result if string isn't empty and doesn't have letters:
    int strLength = snprintf(NULL, 0, "%d", total);
    char * result = malloc(strLength + 1);
    snprintf(result, strLength + 1, "%d", total);
    return result;
  }
  char * result = malloc(2);
  result[0] = 'a';
  result[1] = '\0';
  return result;
}

int main(int argc, char * argv[]) {
//SETTING UP THE GAME: 
//First, read in the file (code taken from https://stackoverflow.com/questions/3501338/c-read-file-line-by-line): 
FILE * fp;
char * line = NULL;
size_t len = 0;
ssize_t readin;

fp = fopen("hangman_words.txt", "r");
if (fp == NULL)
    exit(EXIT_FAILURE);

char wordsToGuess[10][14]; //allocated for the words in the file 
unsigned int indexer = 0; 
while ((readin = getline(&line, &len, fp)) != -1) {
    strcpy(wordsToGuess[indexer], line);
    indexer++; //increase indexer to move to next spot
} 

//At this point, all words are loaded into wordsToGuess, and this can be randomly indexed later. 

int amountOfGames = 0; //have a variable keeping track of the amount of connections 


//Now, we begin listening for the client and setting all that up
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");
  bzero((char * ) & serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr * ) & serv_addr,
      sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);
  //AT THIS POINT EVERYTING FOR SETTING UP THE CONNECTION IS DONE, WE ARE NOW LISTENING FOR A CONNECTION TO START THE GAME

  while (1) { //This will allow for the TCP connection to remain open
    newsockfd = accept(sockfd,
      (struct sockaddr * ) & cli_addr, &
      clilen);
    if (newsockfd < 0)
      error("ERROR on accept");
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");
    //buffer[strlen(buffer) - 1] = '\0'; //Clean up buffer
    
    //This keeps track of the amount of games: 
    if(amountOfGames == 3){ //if more than 3 games, print error and close conection 
        n = write(newsockfd, "server-overloaded", 17);
    } else {
        amountOfGames++; 
    }

    // char * returnMessage = sumOfDigits(buffer);
    // if (returnMessage[0] != 'a') {
    //   int lengthOfMessage = strlen(returnMessage);
    //   delay(2000); //Delay is necessary to ensure proper spacing of text
    //   n = write(newsockfd, returnMessage, lengthOfMessage);
    //   if (n < 0) error("ERROR writing to socket");
    //   while (lengthOfMessage > 1) { 
    //     //Continue to write while the message length is greater than 1
    //     returnMessage = sumOfDigits(returnMessage);
    //     lengthOfMessage = strlen(returnMessage);
    //     delay(2000); //Delay is necessary to esnure proper spacing of text
    //     n = write(newsockfd, returnMessage, lengthOfMessage);
    //     if (n < 0) error("ERROR writing to socket");
    //   }
    // } else {
    //   //Output the error message if letters or empty space is found
    //   n = write(newsockfd, "Sorry, cannot compute!", 22);
    //   if (n < 0) error("ERROR writing to socket");
    // }
    //Close connection: 
    close(newsockfd);
  }
  return 0;
}
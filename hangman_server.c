#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <time.h>
#include <ctype.h>

//Definitions for Array Size
#define LSIZ 128 
#define RSIZ 16 
char line[RSIZ][LSIZ];

int amountOfGames = 0; //have a variable keeping track of the amount of connections 

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

char* generateSpaces (int amountOfSpaces){
    char* spaces = malloc((amountOfSpaces*2)+1); //enough room for each '_' each ' ' and the null terminator
    spaces[0] = '_';
    for(int i = 1; i < amountOfSpaces; i++){
        strcat(spaces, " _");
    }
    return spaces;
}

void checkLetter (char givenLetter, char* guessThis, char* spaces, char* incorrectlettersArray, int* amountOfGuesses){
    if(*amountOfGuesses >= 6) return;
    int changed = 0;
    for(int i = 0; i < (strlen(spaces)); i+=2){
        if(givenLetter == guessThis[i/2]){ //this is the right letter at the right space
            spaces[i]=givenLetter; //set that place to given letter
            changed = 1; //set flag to true;
        }
    }
   //if not changed, that means this letter was wrong so we add it to the wrong letters list
   if(changed == 0){
     if(strlen(incorrectlettersArray) == 0) strncat(incorrectlettersArray, &givenLetter, 1);
     else{
      strcat(incorrectlettersArray, " ");
      strncat(incorrectlettersArray, &givenLetter, 1);
     }
     *amountOfGuesses = *amountOfGuesses+1; //update the amount of guesses
   }
}

//OVERALL FUNCTION FOR GAME
void startGame(int *amountOfGames, int i, char* buffer, int n, int newsockfd){
          amountOfGames++; //else, increment the amount of games      
          //NOW BEGIN THE GAME LOGIC:
          //First, choose a word: 
          //Also, make sure there's a random seed for the random int generator: 
          int r = rand() % (i); //should be a number between 0 and i amount (taken from https://stackoverflow.com/questions/822323/how-to-generate-a-random-int-in-c)
          char* guessThis = line[r];
          char* spaces = generateSpaces(strlen(guessThis)); //accounting for strlen() miscount
          //Have an incorrectLettersArray to keep track of wrong guesses: 
          char* incorrectlettersArray = malloc(16); //because 6 max wrong guesses
          int amountOfGuesses = 1; //have a variable keeping track of guesses per client
          //While there is still something to read on the buffer:
          while (strlen(buffer) > 0) { 
            if(buffer[0] == '\n') amountOfGames--; //this was sent over by a client disconnecting
            //check how many (incorrect) guesses they are on 
            if(amountOfGuesses == 6){
              //we want to end the game and close the connection 
              bzero(buffer, 256); //clear buffer before writing
              strcpy(buffer,"9"); //add in 9 as the flag to buffer; to signify game over 
              strcat(buffer,">>>"); 
              strcat(buffer,"The word was ");
              strcat(buffer, guessThis); 
              strcat(buffer,"\n"); //add in a space
              strcat(buffer,">>>"); 
              strcat(buffer,"You Lose!"); //add in the game over message;
              strcat(buffer,"\n"); //add in a line break
              strcat(buffer,">>>"); 
              strcat(buffer,"Game Over!"); //add in the game over message;
              n = write(newsockfd, buffer, strlen(buffer)); //write out game over 
              bzero(buffer, 256); 
            }
          //be prepared to send out errors in case of it being wrong
          if((buffer[0] == '2' || (isalpha(buffer[1]) == 0) ) && buffer[1] != ' '){
              bzero(buffer, 256); //clear buffer before writing 
              strcpy(buffer,"30"); //add in 30 as the flag to buffer for message length;
              strcat(buffer,">>>"); 
              strcat(buffer, "Error! Please guess one letter."); //add in the error message
              n = write(newsockfd, buffer, strlen(buffer)); //prints out the spaces
              bzero(buffer, 256); //clear out buffer after writing
          } else {
              //Here we check if the given letter is correct or not:
              if(buffer[1] != ' ') checkLetter(buffer[1], guessThis, spaces, incorrectlettersArray, &amountOfGuesses);
              //check if everything is correct and if so print out appropriate message
              int allCorrect = 1;
              for(int i = 0; i < (strlen(spaces)); i++){
                if(spaces[i] == '_'){ //this is the right letter at the right space
                    allCorrect = 0; //it's not finished yet cause there are still spaces
                }
              }
              if(allCorrect == 1){
                bzero(buffer, 256); //clear buffer before writing
                strcpy(buffer,"8"); //add in 9 as the flag to buffer; to signify game over 
                strcat(buffer,">>>"); 
                strcat(buffer,"The word was ");
                strcat(buffer, guessThis); 
                strcat(buffer,"\n"); //add in a space
                strcat(buffer,">>>"); 
                strcat(buffer,"You Win!"); //add in the game over message;
                strcat(buffer,"\n"); //add in a line break
                strcat(buffer,">>>"); 
                strcat(buffer,"Game Over!"); //add in the game over message;
                n = write(newsockfd, buffer, strlen(buffer)); //write out game over 
                bzero(buffer, 256);    
                //end the conenction:
                close(newsockfd);    
                //decrease the client count because a connection ended:
                amountOfGames--;   
              } else{
                //else keep on printing out the other stuff 
                bzero(buffer, 256); //clear buffer before writing 
                strcpy(buffer,"0"); //add in 0 as the flag to buffer;
                char wordLength[3];
                sprintf(wordLength, "%lu", strlen(spaces)); //convert word length to integer
                strcat(buffer, wordLength); //add in the word length 
                char incorrectLength[2];
                sprintf(incorrectLength, "%lu", strlen(incorrectlettersArray)); //convert incorrect length to integer
                strcat(buffer, incorrectLength); //add in the word length 
                strcat(buffer,">>>"); 
                strcat(buffer, spaces); //add in the spaces
                strcat(buffer, "\n"); //add in a space
                strcat(buffer,">>>"); 
                strcat(buffer, "Incorrect Guesses: "); //add in second line 
                strcat(buffer, incorrectlettersArray); //add in incorrect guesses
                n = write(newsockfd, buffer, strlen(buffer)); //prints out the spaces
                bzero(buffer, 256);
              }//end of else  
          }
          //Continue reading: 
          delay(1000);
          n = read(newsockfd, buffer, 255);
          //if (n < 0) error("ERROR reading from socket in inner while loop");
          } //end of while loop
}


int main(int argc, char * argv[]) {
//SETTING UP THE GAME: 
//Add in a random number
srand(0); 

//First, read in the file (code taken from https://www.w3resource.com/c-programming-exercises/file-handling/c-file-handling-exercise-4.php): 
    FILE *fptr = NULL; 
    int i = 0;

    fptr = fopen("hangman_words.txt", "r");
    while(fgets(line[i], LSIZ, fptr)) {
        line[i][strlen(line[i]) - 1] = '\0';
        i++;
    }

//At this point, all words are loaded into wordsToGuess, and this can be randomly indexed later. 

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

 //This will allow for the TCP connection to remain open:
  while (1) { 
    newsockfd = accept(sockfd,
      (struct sockaddr * ) & cli_addr, &
      clilen);
    if (newsockfd < 0)
      error("ERROR on accept");
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255); //read in from the new connection, expecting a "7" to probe the network
    if (n < 0) error("ERROR reading from socket");
    //First listen for connections probing for space:
    if(buffer[0] == '7'){
        if(amountOfGames >= 3){ //if more than 3 games, print error and close conection 
          bzero(buffer, 256); 
          strcpy(buffer,"1"); //add in 1 as the flag to buffer; to signify server overload 
          strcat(buffer,">>>server-overloaded"); 
          n = write(newsockfd, buffer, strlen(buffer));
          bzero(buffer, 256);
       }else{
        bzero(buffer, 256); 
        strcpy(buffer,"5"); //add in 5 as the flag to buffer; to signify no server overload 
        n = write(newsockfd, buffer, strlen(buffer));
        bzero(buffer, 256); //clear the buffer and re-read
        n = read(newsockfd, buffer, 255); //read in from the new connection, expecting a "0" to start the game
        if(buffer[0] == '0'){ //only start the game once you got that 
          //This keeps track of the amount of games: 
            //function to run the game
            startGame(&amountOfGames, i, buffer, n, newsockfd); 
        }  //end of 0 signal to start the game
       }
    }//end of connection probe for space
    
    //Close connection and decrement games:
    //close(newsockfd);
    amountOfGames--; 
  } //end of large infinite while loop
  return 0;
}
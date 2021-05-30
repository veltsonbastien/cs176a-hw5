CC = gcc
ARGS = -Wall


# Compiling all the dependencies
all: hangman_server hangman_client

hangman_server: hangman_server.c 
	$(CC) $(ARGS) -o hangman_server hangman_server.c -lpthread

hangman_client: hangman_client.c
	$(CC) $(ARGS) -o hangman_client hangman_client.c -lpthread

clean:
	rm -f *.o hangman_server hangman_client *~
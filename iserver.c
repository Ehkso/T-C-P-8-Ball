/* This is the Magic 8-ball server file, which receives the client's questions and sends the responses.
 * Made by Austen II, GitHub link: https://github.com/Ehkso
 * I made this file by repurposing an old quiz program I made a year ago, since both consist of questions and answers.
 * I accidentally deleted this file and lost everything when I finshed it, so now I'm rewriting it even faster B) */
// SERVER PROGRAM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include "responses.h"
/* Hardcoded values for server IP and port number; you can modify these or even ignore them altogether by following the instructions written further below */
#define SERVERIP "127.0.0.1"
#define BACKLOG 10
#define BUFSIZE 128
#define PORTNUM 2343
/* Since this server handles clients iteratively, I figured it'd be prudent to set a maximum number
 * of questions that any one client can ask before it moves on to the next one.
 * This is to avoid a user "hogging the Ball", so to speak.
 * I mean, it's pretty pointless if I'm the only one using it, but it'd be useful in other circumstances. */
#define QNUM 5

/* Sends message across TCP socket, takes cfd and buffer as arguments as a result of how I interpreted the code
 * Works well enough for me, though I'm always open to alternatives if anyone would like to show me :) */
void send_msg(int cfd, char* buf){
	size_t totWritten;
	const char* bufw = buf;
	for (totWritten = 0; totWritten < BUFSIZE; ){
		ssize_t numWritten = write(cfd, bufw, BUFSIZE - totWritten);
		if (numWritten <= 0){
			if (numWritten == -1 && errno == EINTR){
				continue;
			} else {
				fprintf(stderr, "Write error. Errno %d.\n", errno);
			}
		}
		totWritten += numWritten;
		bufw += numWritten;
	}
}

/* Receives message from TCP socket, takes same arguments as sending function
 * Returns message to be written into local variables rather than simply printing here and returning void
 * This is so I can perform operations on the received messages and process them */
char* read_msg(int cfd, char* buf){
	size_t totRead;
        char* bufr = buf;
        for (totRead = 0; totRead < BUFSIZE; ) {
		ssize_t numRead = read(cfd, bufr, BUFSIZE - totRead);
		if (numRead == 0)
			break;
		if (numRead == -1) {
			if (errno == EINTR)
				continue;
			else {
				fprintf(stderr, "Read error.\n");
			}
		}
		totRead += numRead;
		bufr += numRead;
	}
	//printf("%s\n", buf);
	return buf;
}

/* The functions you see below for string processing are what I made when this was a mere quiz program.
 * However, there could be plenty of uses for recording client questions:
 * Remembering past questions to give the same response (to avoid conflicting answers when a question is asked twice, for example)
 * Creating a study in which you note what people ask the most about (love, fortune, etc)
 * And plenty more!
 * There are plenty of things you can do with data, but in almost all cases you'd need a way to process, format, and normalize inputs.
 * Thus, I'll leave these functions in case anyone wants to use them. */

/* Trims string, used for trimming the answers received from the client */
char* trim(char* str){
	int i = 0;
	char trim[BUFSIZE];
	while(str[i] != '\n'){
		//printf("str[%d]: %c\n", i, str[i]);
		trim[i] = str[i];
		i++;
	}
	trim[i] = '\0';
	//printf("trim: '%s'\n", trim);
	strcpy(str, trim);
	//printf("Trimmed: '%s'\n", str);
	return str;
}

/* Converts all uppercase characters to lowercase
 * Made so that I don't have to concern myself with case sensitivity */
char* toLower(char* str){
	int i = 0;
	char new[BUFSIZE];
	char lo[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
	char hi[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
	while (str[i] != '\0'){
		for (int j=0;j<26;j++){
			if (str[i] == hi[j]){
				//printf("str[%d]: %c -> %c\n", i, str[i], lo[j]);
				new[i] = lo[j];
				j = 30; // Pushes j out of bounds of for loop; I didn't want to use break.
			} else { new[i] = str[i]; }
		}
		//printf("new[%d] = %c\n", i, new[i]);
		i++;
	}
	new[i] = '\0';
	//printf("lower: '%s'\n", new);
	strcpy(str, new);
	//printf("lowered: '%s'\n", new);
	return str;
}

void close_connection(int cfd){
	/* Close connection */
	if (close(cfd) == -1){
		fprintf(stderr, "close error.\n");
		exit(EXIT_FAILURE);
	}
}

void close_listener(int lfd){
	/* Close listening socket */
	if (close(lfd) == -1){
		fprintf(stderr, "close error.\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]) 
{
    struct sockaddr_in serverAddress;
    
    memset(&serverAddress, 0, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET;
    
    // This allows code to be run with or without specifying server IP and portnum as commandline arguments
    if (argc == 3){
	serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddress.sin_port = htons(atoi(argv[2]));
        fprintf(stdout, "Attempting to listen on <%s:%s>\n", argv[1], argv[2]);
    } else if (argc == 1){
	serverAddress.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddress.sin_port = htons(PORTNUM);
	char ip[] = SERVERIP;
        fprintf(stdout, "Attempting to listen on <%s:%d>\n", ip, PORTNUM);
    } else{
        fprintf(stderr, "Usage: %s <IP address for server> <port number>\n", argv[0]);
        exit(-1);
    }
    
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
       fprintf(stderr, "socket() error.\n");
       exit(-1);
    }

    /*
     * This socket option allows you to reuse the server endpoint
     * immediately after you have terminated it.
     */
    int optval = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
                   &optval, sizeof(optval)) == -1)
       exit(-1);

    int rc = bind(lfd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));
    if (rc == -1) {
       fprintf(stderr, "bind() error.\n");
       exit(-1);
    }

    if (listen(lfd, BACKLOG) == -1)
       exit(-1);


    for (;;) /* Handle clients iteratively */
    {

        fprintf(stdout, "<waiting for clients to connect>\n");
        fprintf(stdout, "<ctrl-C to terminate>\n");

        struct sockaddr_storage claddr;
        socklen_t addrlen = sizeof(struct sockaddr_storage);
        int cfd = accept(lfd, (struct sockaddr *)&claddr, &addrlen);
        if (cfd == -1) {
           continue;   /* Print an error message */
        }

        {
           char host[NI_MAXHOST];
           char service[NI_MAXSERV];
           if (getnameinfo((struct sockaddr *) &claddr, addrlen,
                     host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
              fprintf(stdout, "Connection from (%s, %s)\n", host, service);
           else
              fprintf(stderr, "Connection from (?UNKNOWN?)");
        }


	/* Sends introduction to client */
	char buf[BUFSIZE];
	char* received;
	char* send = "Greetings, young one.\n";
	send_msg(cfd, send);
	snprintf(buf, BUFSIZE, "You may ask up to %d questions, even if the response is inconclusive.\n", QNUM);
	send = buf;
	send_msg(cfd, send);
	send = "May you find the guidance you seek.\n";
	send_msg(cfd, send);

	/* Gets input from client, read locking comment below for explanation */
	send = "Enter 'Y' when you're ready, or 'q' to quit.";
	send_msg(cfd, send);
	send = "Unlock";
	send_msg(cfd, send);
	received = "Lock";
	while (strcmp(received, "Y") != 0 && strcmp(received, "q") != 0){
		
		received = read_msg(cfd, buf);
		received = trim(received);
		if (strcmp(received, "Y") != 0 && strcmp(received, "q") != 0){
			send = "Enter 'Y' to start the quiz, or 'q' to quit.";
			send_msg(cfd, send);
			send = "Unlock";
			send_msg(cfd, send);
		}
	}
	printf("Received: '%s'\n", received);
	if (strcmp(received, "q") == 0){
		send = "End";
		send_msg(cfd, send);
		close_connection(cfd);
		continue;
	}

	
	for (int i=0; i<QNUM; i++){

		if ((i % 10) == 0){
			snprintf(buf, BUFSIZE, "Your %dst question:", i+1);
			send = buf;
			send_msg(cfd, send);
		} else if ((i % 10) == 1){
			snprintf(buf, BUFSIZE, "Your %dnd question:", i+1);
			send = buf;
			try{
				send_msg(cfd, send);
			}catch (Exception e){
				printf("Error");
				close_connection(cfd);
			}
		} else if ((i % 10) == 2){
			snprintf(buf, BUFSIZE, "Your %drd question:", i+1);
			send = buf;
			send_msg(cfd, send);
		} else{
			snprintf(buf, BUFSIZE, "Your %dth question:", i+1);
			send = buf;
			send_msg(cfd, send);
		}
				
		/* Rudimentary locking mechanism to try and coordinate whether the server is waiting for client or vice versa */
		/* I made this up, I have no idea what the proper method is but this works well enough for my intentions */
		// Unlocks client that was waiting for question
		send = "Unlock";
		send_msg(cfd,send);
		// Locks server until it receives an answer from the client
		received = "Lock";
		while(strcmp(received, "Lock") == 0){
			received = read_msg(cfd, buf);
		}
		received = trim(received);
		received = toLower(received);
		printf("Received question: '%s' (post-formatting)\n", received);

		char* respond = (char*)malloc(BUFSIZE * sizeof(char));
		srand(time(NULL));

		int r = random() % 20;
		strcpy(respond, Response[r]);
		snprintf(buf, BUFSIZE, "The Ball replies: %s.\n", respond);
		send = buf;
		send_msg(cfd, send);

		free(respond);
	}
	send = "End";
	send_msg(cfd, send);

	send = "That was your final question.\nGoodbye, and good luck.\n";
	send_msg(cfd, send);

       close_connection(cfd); 
    }

   close_listener(lfd); 

    exit(EXIT_SUCCESS);
}

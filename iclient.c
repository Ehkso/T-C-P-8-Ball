/* This is the Magic 8-ball client file, which sends the client's questions and receives the responses.
 * Made by Austen II, GitHub link: https://github.com/Ehkso
 * Feel free to play with it, but this could've been done completely as one file rather than using sockets
 * and thus if you want a slightly different 8-ball to play with please check out my GiHub - I made a
 * Python 8-ball as a submission to my friend's Awesome-Programming-Challenges repo that only uses one file,
 * doesn't concern itself with ports, *and* has a fun little animation while the 8-ball "thinks" of a response! */
// CLIENT PROGRAM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
/* These are the hardcoded values for server IP and port number, both of which you can change to suit your needs */
#define BUFSIZE 128
#define PORTNUM 2343
#define SERVERIP "127.0.0.1"

/* Sends message across TCP socket, takes cfd and buffer as arguments
 * Maybe there's a better way to do this, but this works pretty well for my purposes anyway */
void send_msg(int cfd, char* buf){
	size_t totWritten;
	const char* bufw = buf;
	for (totWritten = 0; totWritten < BUFSIZE; ){
		ssize_t numWritten = write(cfd, bufw, BUFSIZE - totWritten);
		if (numWritten <= 0){
			if (numWritten == -1 && errno == EINTR){
				continue;
			} else {
				fprintf(stderr, "Write error. Errno :d.\n", errno);
			}
		}
		totWritten += numWritten;
		bufw += numWritten;
	}
}
/* Receives message from TCP socket, takes same arguments as sending function
 * Returns message to be written into local variables rather than simply printing here and returning void
 * This lets me do stuff to the message before I print it (which I do in the server file, and I keep it this way here to mirror it) */
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
				fprintf(stderr, "Read error, Errno %d.\n", errno);
			}
		}
		totRead += numRead;
		bufr += numRead;
       	}
       	//printf("%s\n", buf);
	return buf;
}

int main(int argc, char *argv[]) 
{
    struct sockaddr_in serverAddress;
   
    memset(&serverAddress, 0, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET;
   
   // This allows the code to be run without commandline arguments for speed, or specified when necessary 
    if (argc == 3){
        serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
        serverAddress.sin_port = htons(atoi(argv[2]));
    } else if (argc == 1){
        serverAddress.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddress.sin_port = htons(PORTNUM);
    } else{
        fprintf(stderr, "Usage: %s <IP address of server> <Port number>\n", argv[0]);
	exit(-1);
    }

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1) {
       fprintf(stderr, "socket() error.\n");
       exit(-1);
    }

    int rc = connect(cfd, (struct sockaddr *)&serverAddress, 
                     sizeof(struct sockaddr));
    if (rc == -1) {
       fprintf(stderr, "connect() error, errno %d.\n", errno);
       exit(-1);
    }

    {
        char buf[BUFSIZE];
	char* received;
	char* send;
	
	do{	
		/* This is a rudimentary locking mechanism that I made up to make it work lol
		 * Ironically, about a week or two after I made this I learnt about semaphores and locks
		 * which is basically what this is (albeit a more primitive version of one) */	
		received = "";	
		do{
			printf("%s\n", received);
			received = read_msg(cfd, buf);
		} while ((strcmp(received, "Unlock") != 0) && (strcmp(received, "End") != 0));
		// Server unlocks client after sending question, and locks itself until client sends an answer
		if (strcmp(received, "End") != 0){
			fgets(buf, BUFSIZE, stdin);
			send = buf;
			send_msg(cfd, send);
		}
	} while (strcmp(received, "End") != 0);

	received = read_msg(cfd, buf);
	printf("%s\n", received);
    }

    if (close(cfd) == -1) /* Close connection */
    {
        fprintf(stderr, "close error.\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

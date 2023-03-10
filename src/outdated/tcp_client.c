//--------------------------------------------------------------
// Author: Dietrich Lachmann
// Program: serial_read.c
// Project: AR Structural Testing
// Affiliation: Oregon State University
//--------------------------------------------------------------

// Header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef BUFFER
#define BUFFER 64
#endif

void display_perror_error(char* p_message);

int main(int argc, char* argv[]){
	// socket variables
	int sock_fd;
	int t_port;
	int message;
	struct sockaddr_in server_address;
	struct hostent *server;
	char* host;
	char buff[BUFFER];

	// command line check
	if(argc != 3){
		fprintf(stderr, "usage: %s <hostname> <port>\n", argv[0]);
	}
	
	// assign hostname and port from cli
	host = argv[1];
	t_port = atoi(argv[2]);

	// socket setup
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		display_perror_error("Error creating socket");
	}

	// server setup
	server = gethostbyname(host);
	if(server == NULL){
		fprintf(stderr, "Cannot find host: %s\n", host);
		exit(0);
	}
	bzero((char*) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char*) server->h_addr, (char*) &server_address.sin_addr.s_addr, server->h_length);
	server_address.sin_port = htons(t_port);

	// server connect
	if(connect(sock_fd, &server_address, sizeof(server_address)) < 0 ){
		display_perror_error("Error connecting to server");
	}

	// test shit can be cleaned up
	printf("Please enter message: ");
    bzero(buff, BUFFER);
    fgets(buff, BUFFER, stdin);

	// server write
	message = write(sock_fd, buff, strlen(buff));
	if(message < 0){
		display_perror_error("Error writing to socket");
	}

	// more test shit
	bzero(buff, BUFFER);
    message = read(sock_fd, buff, BUFFER);
    if (message < 0) 
    	display_perror_error("Error reading from socket");
    printf("Echo from server: %s", buff);

	close(sock_fd);
	return 0;
}

void display_perror_error(char* p_message){
	perror(p_message);
	exit(0);
}

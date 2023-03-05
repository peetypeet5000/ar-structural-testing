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
#include <time.h>

#ifndef BUFFER
#define BUFFER 64
#endif

#define NUM_SENSORS 5			//number of values code will simulate, DONT FORGET TO INCREASE BUFFER SIZE IF USING LARGER NUMBERS
#define DELAY 1					//delay in seconds between sending values
#define MAX_VALUE 10			//maximum value (exclusive) generated
#define PRINT_BOOL 1            //print in console what is being sent

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

	srand(time(NULL));  
    float a = MAX_VALUE; //max num


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

	while(1) {

		// test shit can be cleaned up
		/*printf("Please enter message: ");
    		bzero(buff, BUFFER);
    		fgets(buff, BUFFER, stdin);
		*/

		//TESTING VALUES
		bzero(buff, BUFFER);
		int i;
		int pos = 0;
		float x;
        for (i = 0; i < NUM_SENSORS; i++){
            x = (float)rand()/(float)(RAND_MAX/a);
            pos += sprintf(&buff[pos], "%1.4f;", x);
            
        }
		// server write
		message = write(sock_fd, buff, strlen(buff));
		if(message < 0){
			display_perror_error("Error writing to socket");
		}
		switch(PRINT_BOOL){
			case 0:
				break;
			case 1:
				sprintf(&buff[pos], "\n");
        		printf(buff);  
		}
        sleep(DELAY);

	}


	close(sock_fd);
	return 0;
}

void display_perror_error(char* p_message){
	perror(p_message);
	exit(0);
}

//--------------------------------------------------------------
// Author: Dietrich Lachmann
// Program: serial_read.c
// Project: AR Structural Testing
// Affiliation: Oregon State University
//--------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef BUFFER
#define BUFFER 1024
#endif

#ifndef OPEN_PORT
#define OPEN_PORT "/dev/ttyUSB0"
#endif

// change to 0 to disable debug mode
#ifndef DEBUG_MODE
#define DEBUG_MODE 1
#endif


//-------------------------------------------------------------- 
// FUNCTION PROTOTYPES
//--------------------------------------------------------------

void display_errno_error(int errno_int, char* errno_function);
void display_non_errno_error(char* e_message);
void set_termios_flags(struct termios* t);
void read_serial_data(int s_port); 
void display_perror_error(char* p_message);


//-------------------------------------------------------------- 
// MAIN
//--------------------------------------------------------------

int main(int argc, char* argv[]){
	// initialize port here
	int port = 0;
	// count variable to test port
	int count = 0;
	int bytes_received = 0;
	char read_buffer[BUFFER];
	int i = 0;
	// networking stuff
	int sock_fd;
	int t_port;
	int message;
	struct sockaddr_in server_address;
	struct hostent *server;
	char* host;
	char buff[BUFFER];

	int message_from_server = 0;

	// command line checks will be useful later for debugging
	if(argc != 3){
		fprintf(stderr, "usage: %s <hostname> <port>\n", argv[0]);
	}

	// NETWORKING
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
	// END NETWORKING 


	// try to open the port and check for errors	
	port = open(OPEN_PORT, O_RDWR | O_NOCTTY);
	// attempt to open port 20 times before quiting
	while(port < 0 && count < 3){
		count++;
		// error messages
		display_non_errno_error("Retrying serial port connection...");
		if(DEBUG_MODE){
			printf("Connections attempted: %d\n", count);
			display_errno_error(errno, "open");
		}
	}

	// initialize tty of type termios and read in current port data
	struct termios tty;
	memset(&tty, 0, sizeof(tty));
	if(tcgetattr(port, &tty) !=0){
		display_errno_error(errno, "tcgetattr");
		if(!DEBUG_MODE){
			return 1;
		}
	}

	// configure termios port
	set_termios_flags(&tty);
	// configure baud rate in and out
	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);

	// save all flags set above
	if(tcsetattr(port, TCSANOW, &tty) != 0){
		display_errno_error(errno, "tcsetattr");
		if(!DEBUG_MODE){
			return 1;
		}
	}

//	fcntl(port, F_SETFL, O_NONBLOCK);
	
	while(1){
		printf("JHIFEWUIFNEWBFIWEF\n");
		// clear buffer
        	memset(&read_buffer, '\0', sizeof(read_buffer));
		//bzero(read_buffer, BUFFER);
        	// read data in from serial port
        	bytes_received = read(port, read_buffer, sizeof(read_buffer));
		// check bytes recieved
        	if(bytes_received < 0){
        	        display_errno_error(errno, "Error reading in while loop");
			exit(0);
        	}
		else if(bytes_received == EAGAIN || bytes_received == EWOULDBLOCK){
			printf("INSIDE THE ERROR STATEMENT\n");
		}
        	else{
			read_buffer[bytes_received] = 0;
        	        //printf("Received %d: %s\n", bytes_received, read_buffer);
			printf("%s", read_buffer);
			
			message = write(sock_fd, read_buffer, BUFFER);
			if(message < 0){
				display_perror_error("Error writing to socket");
			}


			//memset(&read_buffer[0], '\0', 1);
			//if(send(sock_fd, read_buffer, sizeof(read_buffer), 0) < 0){
			//	printf("Error sending message\n");
			//	return -1;
			//}


			if(recv(sock_fd, read_buffer, BUFFER, 0) < 0){
				printf("Error reading from server\n");
				return -1;
			}
			printf("Response: %s\n", read_buffer);
        	}
	}

	// close serial port
	close(port);
	close(sock_fd);

	return 0;
}


//-------------------------------------------------------------- 
// FUNCTIONS
//--------------------------------------------------------------

//-------------------------------------------------------------- 
// Function: display_errno_error
// Purpose: catches errno at specific error points and displays
// information about the error
//--------------------------------------------------------------
void display_errno_error(int errno_int, char* errno_function){
	printf("Error %i from %s: %s\n", errno_int, errno_function, strerror(errno_int)); 
}

//-------------------------------------------------------------- 
// Function: display_non_errno_error
// Purpose: error handling NOT at the system level
//--------------------------------------------------------------
void display_non_errno_error(char* e_message){
	printf("Error message: %s\n", e_message);
}


void display_perror_error(char* p_message){
	perror(p_message);
	exit(0);
}


//-------------------------------------------------------------- 
// Function: set_termios_flags
// Purpose: modify how serial data is being read this is going
// in a function for right now, but moving forward this will
// probably want to live in its own header file for termios
// setup
//--------------------------------------------------------------
void set_termios_flags(struct termios* t){
	// comment/uncomment what you need for termios setup
	
	// parity
	t->c_cflag &= ~PARENB; // clear
//	t->c_cflag |= PARENB; // set

	// stop bits
	t->c_cflag &= ~CSTOPB; // 1 stop bit
//	t->c_cflag |= CSTOPB; // 2 stop bits

	// bits per byte
	t->c_cflag &= ~CSIZE; // clear size bits before set
//	t->c_cflag |= CS5; // 5 bits
//	t->c_cflag |= CS6; // 6 bits
//	t->c_cflag |= CS7; // 7 bits
	t->c_cflag |= CS8; // 8 bits

	// hardware flow control
	t->c_cflag &= ~CRTSCTS; // disable
//	t->c_cflag |= CRTSCTS; // enable

	// turn on CREAD and CLOCAL
	// this should probably never be modifed
	t->c_cflag |= CREAD | CLOCAL;

	// TO DO
	// we disable canonical mode because we are basing read off
	// frequency currently, not string characters
	// this CAN be modified however, I believe that the DAQ allows
	// for canonical write using specific characters, this is 
	// something to revisit

	// canonical mode
	t->c_cflag &= ~ICANON; // disable
//	t->c_cflag |= ICANON; // enable

	// echo
	t->c_cflag &= ~ECHO; // disable
//	t->c_cflag |= ECHO; // echo
	
	// erasure
	t->c_cflag &= ~ECHOE; // disable
//	t->c_cflag |= ECHOE; // enable

	// new line echo
	t->c_cflag &= ~ECHONL; // disable
//	t->c_cflag |= ECHONL; // enable

	// disable signal chars
	// this should also probably not be modified
	t->c_cflag &= ~ISIG;

	// software flow control
	// I am not totally sure what this does but it seems like we
	// want it for serial communication according to this doc:
	// http://uw714doc.sco.com/en/SDK_sysprog/_TTY_Flow_Control.html
	t->c_iflag &= ~(IXON | IXOFF | IXANY);

	// special byte handling
	// this should probably not be modified for serial
	t->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

	// output modes
	// these handle character conversion, so just disable them
	t->c_oflag &= ~OPOST;
	t->c_oflag &= ~ONLCR;

	// TO DO
	// VMIN and VTIME are going to be pretty depended on the speed of the DAQ
	// as far as I know so this needs to be revisited
	
	// timing
	t->c_cc[VTIME] = 5; // TEMP VALUE
	t->c_cc[VMIN] = 0; // TEMP VALUE
}

//-------------------------------------------------------------- 
// Function: read_serial_data
// Purpose: currently read and display data, this may be changed
// in the future though
//--------------------------------------------------------------
/*void read_serial_data(int s_port){
	int bytes_received = 0;
	char read_buffer[32];	

	// clear buffer
	memset(&read_buffer, '\0', sizeof(read_buffer));
	// read data in from serial port
	bytes_received = read(s_port, read_buffer, sizeof(read_buffer) - 1);
	// check bytes recieved
	if(bytes_received < 0){
		display_errno_error(errno, "Error reading in read_serial_data");
	}
	else{
		read_buffer[bytes_received] = 0;
		printf("Received %d: %s\n", bytes_received, read_buffer);
	}
}*/

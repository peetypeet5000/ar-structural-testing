//--------------------------------------------------------------
// Author: Dietrich Lachmann
// Program: ar_serial_client.c
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef OPEN_PORT
#define OPEN_PORT "/dev/ttyUSB0"
#endif

#ifndef BUFFER
#define BUFFER 64
#endif

// change to 0 to disable debug mode
#ifndef DEBUG_MODE
#define DEBUG_MODE 1
#endif

//-------------------------------------------------------------- 
// Function: display_perror_error
// Purpose: error handling for perror messages
//--------------------------------------------------------------
void display_perror_error(char* p_message){
	perror(p_message);
	exit(0);
}

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

//-------------------------------------------------------------- 
// Function: set_termios_flags
// Purpose: modify how serial data is being read this is going
// in a function for right now, but moving forward this will
// probably want to live in its own header file for termios
// setup
//--------------------------------------------------------------
int
set_termios_flags (int fd, int speed, int parity)
{
        struct termios tty;
        if (tcgetattr (fd, &tty) != 0)
        {
                display_errno_error (errno ,"tcgetattr");
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                display_errno_error (errno, "tcsetattr");
                return -1;
        }
        return 0;
}

//-------------------------------------------------------------- 
// Function: set_blocking
// Purpose: controls weather input fd will block upon a read
// or write
//--------------------------------------------------------------
void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                display_errno_error (errno, "tggetattr");
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                display_errno_error (errno, "tcsetattr");
}

//-------------------------------------------------------------- 
// Function: read_serial_data
// Purpose: currently read and display data, this may be changed
// in the future though
//--------------------------------------------------------------
void read_serial_data(int s_port, char dest[BUFFER]){
	int bytes_received = 0;
	char read_buffer [64];

	// clear buffer
	memset(&read_buffer, '\0', sizeof(read_buffer));
	// read data in from serial port
	bytes_received = read(s_port, &read_buffer, sizeof(read_buffer));
	// check bytes recieved
	if(bytes_received < 0){
		display_errno_error(errno, "Error reading in read_serial_data");
	}

	printf("Read %i bytes from serial port. ", s_port);
	printf("Message recieved: %s\n", read_buffer);
	memcpy(dest, read_buffer, BUFFER); 
}


//-------------------------------------------------------------- 
// MAIN
//--------------------------------------------------------------

int main(int argc, char* argv[]){
	// serial variables
	int serial_read_port = 0;
	int count = 0;

	// socket variables
	int sock_fd = 0;
	int tcp_write_port = 0;
	int message;
	struct sockaddr_in server_address;
	struct hostent *server;
	char* host;
	char buff[BUFFER];	

	// command line check
	if(argc != 3){
		fprintf(stderr, "usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}

//-------------------------------------------------------------- 
// SERIAL SETUP BEGIN
//--------------------------------------------------------------
	// try to open the port and check for errors	
	serial_read_port = open (OPEN_PORT, O_RDWR | O_NOCTTY | O_SYNC);;
	// attempt to open port 20 times before quiting
	while(serial_read_port <= 0 && count < 20){
		count++;
		// error messages
		display_non_errno_error("Retrying serial port connection...");
		if(DEBUG_MODE){
			printf("Connections attempted: %d\n", count);
			display_errno_error(errno, "open");
		}
	}

	// initialize tty of type termios and read in current port data
	set_termios_flags (serial_read_port, B9600, 0);

	// configure termios port
	set_blocking(serial_read_port, 1);

	// Read data
	while(1){
		bzero(buff, BUFFER);
		read_serial_data(serial_read_port, buff);
	}

//--------------------------------------------------------------
// SERIAL SETUP END
//--------------------------------------------------------------

//-------------------------------------------------------------- 
// TCP SETUP BEGIN
//--------------------------------------------------------------
	// assign hostname and port from cli
	host = argv[1];
	tcp_write_port = atoi(argv[2]);

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
	server_address.sin_port = htons(tcp_write_port);

	// server connect
	if(connect(sock_fd, &server_address, sizeof(server_address)) < 0 ){
		display_perror_error("Error connecting to server");
	}
//-------------------------------------------------------------- 
// TCP SETUP END
//--------------------------------------------------------------

	while(1){
		bzero(buff, BUFFER);
		read_serial_data(serial_read_port, buff);

	}

	// CLEANUP/TEARDOWN CODE
	close(sock_fd);
	close(serial_read_port);

	return 0;
}

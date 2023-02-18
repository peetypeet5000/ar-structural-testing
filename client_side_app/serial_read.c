//--------------------------------------------------------------
// Author: Dietrich Lachmann
// Program: serial_read.c
// Project: AR Structural Testing
// Affiliation: Oregon State University
//--------------------------------------------------------------

// Useful C header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Useful Linux header files
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#ifndef OPEN_PORT
#define OPEN_PORT "/dev/ttyUSB0"
#endif


//-------------------------------------------------------------- 
// FUNCTION PROTOTYPES
//--------------------------------------------------------------

void display_errno_error(int i, char* s);
void display_non_errno_error(char* s);
void set_termios_flags(struct termios* t);


//-------------------------------------------------------------- 
// MAIN
//--------------------------------------------------------------

int main(int argc, char* argv[]){
	bool debug_flag = false;

	// command line checks will be useful later for debugging
	if(argc > 1){
		display_non_errno_error("Too many arguments supplied!");
		return 1;
	}
	
	// try to open the port and check for errors
	int port = open(OPEN_PORT, O_RDWR);
	if(port < 0){
		display_errno_error(errno, "open");
		if(!debug_flag){
			return 1;
		}
	}	

	// initialize tty of type termios and read in current port data
	struct termios tty;
	if(tcgetattr(port, &tty) !=0){
		display_errno_error(errno, "tcgetattr");
		if(!debug_flag){
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
		if(!debug_flag){
			return 1;
		}
	}

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
void display_errno_error(int i, char* s){
	printf("Error %i from %s: %s\n", i, s, strerror(i)); 
}

//-------------------------------------------------------------- 
// Function: display_non_errno_error
// Purpose: error handling NOT at the system level
//--------------------------------------------------------------
void display_non_errno_error(char* s){
	printf("Error message: %s\n", s);
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


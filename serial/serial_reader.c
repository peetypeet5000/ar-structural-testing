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

#ifndef BUFFER
#define BUFFER 1024
#endif

#ifndef OPEN_PORT
#define OPEN_PORT "/dev/ttyUSB0"
#endif

// Test Values
#define NUM_SENSORS 5			//number of values code will simulate, DONT FORGET TO INCREASE BUFFER SIZE IF USING LARGER NUMBERS
#define MAX_VALUE 10			//maximum value (exclusive) generated
#define TEST_MODE true

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

    // If an argument is provided, run in test mode
    if(!TEST_MODE) {
        // try to open the port and check for errors	
        port = open(OPEN_PORT, O_RDWR | O_NOCTTY);
        // attempt to open port 20 times before quiting
        while(port < 0 && count < 3){
            count++;
            // error messages
            display_non_errno_error("Retrying serial port connection...");
            if(TEST_MODE){
                printf("Connections attempted: %d\n", count);
                display_errno_error(errno, "open");
            }
        }

        // initialize tty of type termios and read in current port data
        struct termios tty;
        memset(&tty, 0, sizeof(tty));
        if(tcgetattr(port, &tty) !=0){
            display_errno_error(errno, "tcgetattr");
            if(!TEST_MODE){
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
            if(!TEST_MODE){
                return 1;
            }
	    }
	} 

	while(1){
		// clear buffer
        memset(&read_buffer, '\0', sizeof(read_buffer));
		//bzero(read_buffer, BUFFER);
        // read data in from serial port if not in test mode
        if(!TEST_MODE) {
            bytes_received = read(port, read_buffer, sizeof(read_buffer));

            if(bytes_received < 0){
                display_errno_error(errno, "Error reading in while loop");
			    exit(0);
            }
		    else if(bytes_received == EAGAIN || bytes_received == EWOULDBLOCK){
			    printf("INSIDE THE ERROR STATEMENT\n");
		    }
        } else {
            // In test mode, generate random numbers
            int pos = 0;
            float x;
            for (int i = 0; i < NUM_SENSORS; i++){
                x = (float)rand()/(float)(RAND_MAX/MAX_VALUE);
                pos += sprintf(&read_buffer[pos], "%1.4f;", x);
                
            }
			read_buffer[pos] = '\n';

            bytes_received = strlen(read_buffer);
        }
    
    
        read_buffer[bytes_received] = 0;
        printf("%s", read_buffer);
		fflush(stdout);

        if(TEST_MODE) {
            //usleep(100000);
        }
	}

	// close serial port
	close(port);

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

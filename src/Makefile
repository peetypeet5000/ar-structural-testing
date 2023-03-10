CC = gcc
CFLAGS = -g -Wall
TARGET = ar_serial_client

all: ar_serial_client.c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -w

clean:
	$(RM) $(TARGET) *.so *.o a.out

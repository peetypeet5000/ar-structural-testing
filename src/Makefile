CC = gcc
CFLAGS = -g -Wall
TARGET = serial_read

all: serial_read.c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

test: $(TARGET).c
	$(CC) $(CFLAGS) -shared -o $(TARGET).so $(TARGET).c

clean:
	$(RM) $(TARGET) *.so *.o a.out

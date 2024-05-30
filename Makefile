CC = gcc
CFLAGS = -g -Wall -Wextra
INCLUDES = -I/usr/include/SDL2
LDFLAGS = -lSDL2
TARGET = emulator
SRCS = $(wildcard src/*.c) 
OBJS = $(SRCS:.c=.o)
HEADERS = init.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)
		
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGET)

debug:
	gdb --args $(TARGET) roms/test_opcode.ch8 

.PHONY: all clean

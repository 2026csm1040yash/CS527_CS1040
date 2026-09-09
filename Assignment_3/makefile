CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = simulator.exe
SRC = main.c compiler.c processor.c memory.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-del /Q $(OBJ) $(TARGET) 2>nul

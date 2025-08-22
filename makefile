# Compiler and flags
CC = gcc
CFLAGS = $(shell sdl2-config --cflags) -Wall -Wextra -g
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lm

# Source files
SRCS = main.c images.c canvas.c controls.c render.c loader.c super.c
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = alnview

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)
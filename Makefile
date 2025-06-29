TARGET = build/main

CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRC = main.c lexer.c parser.c
OBJ = $(SRC:%.c=build/%.o)

FILE ?= input.txt

all: $(TARGET)

# Link object files
$(TARGET): $(OBJ)
	@mkdir -p build
	$(CC) $(OBJ) -o $(TARGET)

# Compile source files into build/
build/%.o: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf build

# Run the program with input file
run: all
	@echo "Running with input file: $(FILE)"
	@./$(TARGET) $(FILE)

TARGET = build/main.o

CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRC = main.c lexer.c parser.c codegenerator.c
OBJ = build/main.o
FILE ?= test.bling

all: run

# Compile source files into build/
comp: $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS)  -o $(OBJ) $(SRC) 

# Clean build artifacts
clean:
	rm -rf build

# Run the program with input file
run: comp
	@echo "Running with input file: $(FILE)"
	@./$(TARGET) $(FILE)

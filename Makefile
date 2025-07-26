TARGET = build/main           # This is the actual binary
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRC = main.c lexer.c parser.c codegenerator.c utils/hashmapoperators.c
FILE ?= test.bling

all: run

# Compile source files into build/
comp: $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Compile with debug symbols (-g)
debug: CFLAGS += -g
debug: clean comp
	@echo "Built with debug symbols. Run with: gdb $(TARGET)"

# Clean build artifacts
clean:
	rm -rf build

# Run the program with input file
run: comp
	@echo "Running with input file: $(FILE)"
	@./$(TARGET) $(FILE)

# Usage:
#   make run FILE=path/to/your_file.s

FILE ?= test.s                     # default file
OBJ  := $(FILE:%.s=build/%.o)
OUT  := build/output

SDKROOT := $(shell xcrun --sdk macosx --show-sdk-path)

all: run

tell:
	echo $(FILE)

buildi: $(FILE)
	echo "$(FILE)"
	as -arch arm64 -o $(OBJ) $(FILE)
	ld -o $(OUT) $(OBJ) \
		-lSystem \
		-syslibroot $(SDKROOT) \
		-e _start 

run: buildi
	./$(OUT)

clean:
	rm -rf build/*

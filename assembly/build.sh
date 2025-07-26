#!/bin/bash
mkdir -p build

as -arch arm64 -o build/generated.o assembly/generated.s

ld -o build/output build/generated.o \
  -lSystem \
  -syslibroot "$(xcrun --sdk macosx --show-sdk-path)" \
  -e _main
./build/output

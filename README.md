# Bling Compiler Project

## Overview

This repository hosts the source code for the Bling Compiler, a project designed to translate source code written in the Bling programming language into assembly code. The compiler is structured into several distinct phases: lexical analysis, parsing, and code generation.

## Features

- **Lexical Analyzer (Lexer):** Responsible for tokenizing the input `.bling` source file. It accurately identifies and categorizes language elements such as keywords, identifiers, operators, separators, and integer literals.
- **Parser:** Consumes the token stream produced by the lexer to construct an Abstract Syntax Tree (AST). The AST serves as an intermediate representation of the source code's syntactic structure.
- **Code Generator (Under Development):** This component is designed to traverse the generated AST and produce corresponding assembly language instructions, specifically targeting the Apple Silicon (ARM64) architecture. This feature is currently undergoing active development and is not yet fully implemented.

## Getting Started

To build and execute the Bling Compiler, follow the instructions below. A `Makefile` is provided to streamline the process.

### Prerequisites

*   A C compiler (e.g., GCC, Clang)
*   `make` utility

### Building the Compiler

Navigate to the root directory of the project and execute the following command:

```bash
make comp
```

### Running the Compiler

To compile a Bling source file, use the `run` target, specifying the input file via the `FILE` variable:

```bash
make run FILE=<your_file.bling>
```

If the `FILE` variable is not provided, the compiler will default to processing `test.bling`.

### Cleaning Build Artifacts

To remove all generated build files and executables, use the `clean` target:

```bash
make clean
```

## Project Structure

- `main.c`: The primary entry point of the compiler, orchestrating file I/O and the invocation of the lexical and parsing phases.
- `lexer.h`, `lexer.c`: Implementation of the lexical analyzer.
- `parser.h`, `parser.c`: Implementation of the parser and AST construction.
- `codegenerator.h`, `codegenerator.c`: Contains the ongoing implementation of the code generator.
- `utils/`: A directory housing various utility functions, including a custom hash map implementation.
- `assembly/`: Contains resources and scripts related to assembly code generation and testing.
- `Makefile`: The project's build automation script.
- `test.bling`: An example Bling source file for testing and demonstration purposes.

## Future Enhancements

- Full implementation and optimization of the code generator to produce robust and functional assembly code.
- Expansion of language features to include support for additional data types (e.g., strings, floating-point numbers).
- Integration of advanced programming constructs, such as functions, control flow statements (e.g., `if`, `while`), and data structures.
- Development of comprehensive error handling and recovery mechanisms within the parser.
- Establishment of a thorough test suite to ensure the correctness and reliability of all compiler components.

## License

This project is licensed under the terms specified in the `LICENSE.md` file.

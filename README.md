# Bling Compiler

This is a simple compiler for the Bling programming language. It takes a `.bling` file as input and is intended to generate assembly code.

## Features

- **Lexer:** The lexer tokenizes the input `.bling` file. It can identify keywords, identifiers, operators, separators, and integers.
- **Parser:** The parser takes the tokens from the lexer and builds an Abstract Syntax Tree (AST).
- **Code Generator (Partial):** The code generator is intended to take the AST and generate assembly code. This feature is not yet fully implemented.

## How to Run

To compile and run the compiler, you can use the provided `Makefile`.

1.  **Compile the compiler:**
    ```bash
    make comp
    ```
2.  **Run the compiler with a `.bling` file:**

    ```bash
    make run FILE=<your_file.bling>
    ```

    If you don't provide a `FILE` argument, it will default to `test.bling`.

3.  **Clean the build artifacts:**
    ```bash
    make clean
    ```

## File Descriptions

- `main.c`: The main entry point for the compiler. It handles file I/O and calls the lexer and parser.
- `lexer.h`, `lexer.c`: The lexical analyzer, which converts the source code into a stream of tokens.
- `parser.h`, `parser.c`: The parser, which builds the Abstract Syntax Tree (AST) from the tokens.
- `codegenerator.h`, `codegenerator.c`: The code generator, which is intended to generate assembly code from the AST.
- `utils/`: This directory contains utility functions, such as the hashmap implementation.
- `assembly/`: This directory contains files related to assembly code generation.
- `Makefile`: The build script for the project.
- `test.bling`: An example `.bling` file for testing.

## Future Scope

- Complete the implementation of the code generator to produce functional assembly code.
- Add support for more data types, such as strings and floats.
- Implement more complex language features, such as functions and control flow statements.
- Add error handling and recovery to the parser.
- Write more comprehensive tests for all components of the compiler.

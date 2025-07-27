💎 Bling Compiler Project (for ARM64 architecture) 🍏

🚀 Overview

Welcome to the Bling Compiler — a powerful toolchain designed to translate Bling source code into optimized Apple Silicon (ARM64) assembly. This compiler is thoughtfully architected and built from the ground up, encompassing three key phases: lexical analysis, parsing, and code generation.

The Bling language is minimalist yet expressive, supporting core constructs for computation. Here’s a snippet that generates a Fibonacci-like sequence:

int x = 0;
int y = 1;
int z = 0;
int i = 1;

while(i neq 100){
x = y;
y = z;
z = y + x;
i = i + 1;
}

exit(x);

⸻

✨ Features
• 🔍 Lexical Analyzer (Lexer):
Converts .bling source files into a stream of tokens — recognizing keywords, identifiers, operators, separators, and literals.
• 🌲 Parser:
Builds an Abstract Syntax Tree (AST) from the token stream, capturing the structural and syntactic essence of the code.
• ⚙️ Code Generator (in progress):
Traverses the AST to produce ARM64 assembly instructions, specifically optimized for Apple Silicon.

⸻

🛠 Getting Started

A Makefile is included to simplify building and running the compiler.

✅ Prerequisites

Ensure you have the following installed:
• 🧰 A C compiler (gcc or clang)
• 📦 make utility

🧱 Building the Compiler

make comp

🏃‍♂️ Running the Compiler

To compile a Bling source file:

make run FILE=<your_file.bling>

If no FILE is specified, it defaults to test.bling.

🧹 Cleaning Up

make clean

Removes all build artifacts and binaries.

⸻

🗂 Project Structure

.
├── main.c # Entry point
├── lexer.{h,c} # Lexical analyzer
├── parser.{h,c} # AST builder
├── codegenerator.{h,c} # ARM64 code generator (in progress)
├── utils/ # Hashmaps and reusable components
├── assembly/ # Apple Silicon assembly tests/resources
├── test.bling # Sample Bling code
└── Makefile # Build automation

⸻

🔮 Future Enhancements
• 🧬 Full ARM64 Code Generation:
Finalize and polish the code generator for complete ARM64 coverage.
• 🧵 Richer Language Support:
Add strings, floats, and other primitive types.
• 🧠 Advanced Control Flow:
Support for if, else, nested loops, and user-defined functions.
• 🛡 Robust Error Handling:
Improve diagnostic messages and recovery mechanisms.
• 🧪 Automated Testing Suite:
Ensure correctness and performance of all components — especially ARM64 output.

⸻

📄 License

This project is open-source and available under the terms of the LICENSE.md file.

⸻

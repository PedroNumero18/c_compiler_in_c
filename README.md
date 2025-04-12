# Simple C Compiler

A basic C compiler written in C that can compile a subset of the C language to ARM assembly.

## Overview

This project implements a minimalist C compiler that translates C source code to ARM assembly. It supports a subset of C language features, focusing on fundamental constructs like functions, variables, expressions, and control flow.

The compiler follows the traditional compilation pipeline:
1. **Lexical Analysis**: Tokenizing the input source code
2. **Syntax Analysis**: Parsing tokens into an Abstract Syntax Tree (AST)
3. **Semantic Analysis**: Checking for semantic errors and building symbol tables
4. **Intermediate Representation**: Generating architecture-independent IR code
5. **Code Generation**: Generating ARM 32-bit assembly code

## Features

The compiler supports:

- Basic data types: `int` and `char`
- Variables and arrays
- Arithmetic and logical expressions
- Control flow statements: `if`, `else`, `while`
- Function definitions and calls
- ARM 32-bit assembly generation
- Command-line options for controlling output

## Command-line Options

```
Usage: ./bin/scc input.c [-o output] [-S] [-t=target]
  -o output   Specify output filename (default: a.out)
  -S          Generate assembly output instead of executable
```

## Limitations

This compiler implements only a subset of C. Notable limitations include:

- No support for structs, unions, or type definitions
- No support for Standard C library
- Limited preprocessor support
- No optimization passes
- Limited error recovery
- No direct executable generation (ARM assembly only)

## Building the Compiler

To build the compiler, use the provided Makefile:

```
make
```

## Usage Example

Here's an example of how to compile a C file to ARM assembly:

```
/CComp test/example.c -S -o example_arm.s
```

The assembly files can then be assembled and linked using appropriate ARM toolchains (with gcc for example).

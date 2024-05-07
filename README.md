# MIPS Assembly Code Generator

Overview

This C++ program is designed to generate MIPS assembly code from a given parse tree. It specifically handles code generation for expressions, statements, declarations, and procedures, following the syntax and semantics of a subset of the WLP4 language (a simplified version of C++ used for educational purposes). The primary purpose of this generator is to assist in the compilation process by converting high-level constructs into low-level MIPS assembly instructions.


Features

Expression Handling: Supports arithmetic and logical operations, including handling of pointers.
Control Structures: Implements if-else, while loops, and procedure calls.
Memory Management: Includes support for dynamic memory allocation and deallocation (new and delete).
Error Reporting: Provides basic error reporting that notifies the user of syntax and semantic issues during the parse tree traversal.
Symbol Table Management: Manages a symbol table to keep track of variable declarations and their corresponding memory locations.
Usage


To use this code generator, you will need to provide a parse tree as input in a specific format. The input should be read from standard input (stdin), and the output (MIPS assembly code) will be printed to standard output (stdout).

Input Format: The input should represent a parse tree where each node describes a grammatical rule applied, the associated lexemes, and type annotations where applicable. The first line should specify the start of the parse tree.

Output: The output is MIPS assembly code that corresponds to the given parse tree. This assembly code can be run on a MIPS simulator such as SPIM or MARS to verify its correctness.




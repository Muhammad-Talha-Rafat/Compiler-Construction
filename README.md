# Compiler Construction

This repository contains a simple **compiler project** built in C++ as part of a **Compiler Construction** course.  
The project demonstrates lexical analysis, parsing, and basic error handling for a subset of the C++ language.

---

## 📌 Features

- **Lexer**:  
  - Tokenizes identifiers, keywords, literals, and operators.  
  - Handles input programs with basic C++ constructs.

- **Parser**:  
  - Implements a grammar-driven recursive descent parser.  
  - Supports declarations, function definitions, loops, conditional statements, assignments, I/O operations, and return statements.  
  - Rejects programs that do not follow the specified grammar.

- **Grammar Rules**:  
  The grammar (defined in [`Grammer.txt`](./Grammer.txt)) includes:
  - Program structure: `headers → declarations → main`  
  - Declarations: variables, functions, objects, `#define`  
  - Control flow: `if/else`, `while`, `do-while`, `for`  
  - Expressions, assignments, increments/decrements  
  - Input/Output: `cin`, `cout`  
  - Functions with arguments, return statements  
  - Blocks and nested statements  

---

## ⚠️ Limitations & Exceptions

As outlined in [`exceptions.txt`](./exceptions.txt):

- Some invalid lexemes are tokenized but deferred for parser error handling (e.g., `7number`).  
- Advanced operations like `sizeof`, `::`, or templates are tokenized but not parsed.  
- Global-scope variable usage is disallowed (only declarations are valid).  
- Default parameter assignments in functions are not supported.  
- Blocks must always use braces `{}` even for a single statement.  
- `return` statements only accept **literal values** (not identifiers or expressions).  
- Conditions in control flow support only **identifiers** and **literals**.  

---

## 📂 Project Structure

```
├── Grammer.txt      # Grammar definition for the language
├── exceptions.txt   # Notes on known exceptions/limitations
├── Tokenizer.h      # Lexer implementation
├── Parser.h         # Parser implementation
├── main.cpp         # Entry point
└── ...              # Other helper classes
```

---

## 🚀 Getting Started

### Prerequisites
- C++17 or later
- g++ / clang++ compiler

### Build
```bash
g++ -std=c++17 -o compiler main.cpp Parser.cpp Tokenizer.cpp
```

### Run
```bash
./compiler input_code.txt
```

---

## 🧩 Example

**Input:**
```cpp
#include <iostream>
using namespace std;

int main() {
    int num = 7;
    if (num > 5) {
        cout << num;
    }
    return 0;
}
```

**Output:**
```
Parsing successful.
```

---

## 📖 Notes
- This is an **educational project** and does not aim to be a complete C++ compiler.  
- The focus is on understanding compiler design concepts: lexical analysis, parsing, and error handling.

---


# 🐍 Zeta — A Python-Like Language Interpreter in C

Zeta is a tiny interpreted programming language, written entirely in C, that mimics the simplicity of Python's syntax. I built it to gain a deep understanding of how interpreters work under the hood—covering everything from lexing and parsing to AST generation and evaluation.

## 🚀 Why I Built This

I wanted to demystify the "magic" behind programming languages like Python. So, I rolled up my sleeves and built one from scratch using only C. Along the way, I explored:

* 🔤 Lexers and tokenization
* 🌲 Abstract Syntax Trees (ASTs)
* 🧠 Recursive descent parsing
* 🔁 Interpreting arithmetic expressions and assignments
* 🧰 Building dynamic arrays (like vectors in C++)
* ⚠️ Error handling and memory safety

## 🛠 Features

✅ Basic arithmetic (`+`, `-`, `*`, `/`)
✅ Variable assignment and usage
✅ Order of operations (via AST parsing)
✅ Scientific notation (e.g. `1.5e3`)
✅ Unary operations (`+x`, `-x`)
✅ Multi-statement execution
✅ Errors with file/line/column awareness
✅ Fully written in ANSI C (mostly C99+)

## 📦 Example Code

Here’s a simple Zeta program:

```zeta
x = 10;
y = x * 2;
z = y + 3;
```

When run through the interpreter:

```bash
$ ./zeta.exe .zeta
20 23 
```

It prints the result of the last evaluated expressions (`y` and `z`).

## 📄 Project Structure

* `lexer.c` — Turns characters into tokens (lexing)
* `parser.c` — Converts tokens into an AST (parsing)
* `interpreter.c` — Traverses AST and evaluates expressions
* `darray.c` — Custom dynamic array implementation
* `zeta.c` — Entry point and REPL/file execution logic

All functionality is implemented in one file for learning purposes.

## 📚 Concepts You’ll Learn

If you're diving into this codebase, you'll come away understanding:

* How interpreters tokenize and parse source code
* How to model abstract syntax trees using C structs
* Visitor pattern in the context of evaluating AST nodes
* Memory management in C with manual allocation and cleanup
* Why languages like Python feel so "magical" to use

## 🧪 How to Run

### 1. Clone the repo:

```bash
git clone https://github.com/0b01010100/Zeta.git
cd Zeta
```

### 2. Compile:

```bash
gcc zeta.c -o zeta
```

### 3. Run Zeta programs:

```bash
./zeta.exe .zeta
```

## ✏️ Todo

* [ ] Add support for `if` statements and loops
* [ ] Create a REPL mode (interactive shell)
* [ ] Add support for functions
* [ ] Improve error messages with line numbers
* [ ] Add unit tests

## 💡 Inspirations

* [Crafting Interpreters](https://ruslanspivak.com/lsbasi-part7/) by Ruslan Spivak

* Python’s CPython source code
* Lua’s minimalism and simplicity
* Writing interpreters from scratch (as a rite of passage!)

## 🧠 Final Thoughts

Zeta isn't meant to compete with real languages—it's a stepping stone for me to learn more about interpreters.
---

**MIT License**
Made with ❤️ in C


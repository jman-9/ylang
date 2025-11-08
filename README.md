# ylang 0.0.1 — *it runs*

A lightweight, C-like, retro-styled programming language

---

## Overview

`ylang` is a lightweight, C-like language designed with a simple and retro style.

It implements a complete compilation pipeline — from scanning to bytecode generation — and execution on its own virtual machine, named `yvm` — keeping it small, structured, and clear.



## Architecture

**Pipeline:**

```
Source (.y)
 → Scanner
 → Parser
 → Semantic Analyzer
 → Bytecode Builder
 → Machine (VM)
```

**VM Features:**

* Variable & arithmetic operations
* Control statements: if/for/break/continue/return
* User functions
* Dynamically typed variables



## Project Structure

```
ylang/
├── core/                 # ylang core - Scanner, Parser, Semantic, Bytecode, etc
│   └── vm/               # yvm - Machine 
├── tests/                # test programs
├── tools/                # tools
│   └── ylang/            # ylang CLI
└── examples/             # Example scripts (.y)

```



## Build Instructions

### Prerequisites

* CMake ≥ 3.16
* C++20 compatible compiler
* Visual Studio 2022 (Windows) or GCC/Clang (Linux)

### Windows (Visual Studio 2022)

#### Using GUI
```
1. Open the project folder in Visual Studio 2022
2. Select configuration
3. Build
4. Run
```

#### Using CLI

```cmd
cmake --preset x64-debug
cmake --build --preset x64-debug
```

### Linux / macOS

```bash
mkdir build && cd build
cmake ..
make
```



## Usage

Run with source file:

```bash
ylang examples/hello.y
```

Example result:

```
Hello, ylang!
```



## Examples

| File                                  | Description           |
| ------------------------------------- | --------------------- |
| [hello.y](examples/hello.y)           | Basic print           |
| [cond.y](examples/cond.y)             | Conditional test      |
| [forloop.y](examples/forloop.y)       | Simple loop & sum     |
| [forctrl.y](examples/forctrl.y)       | Break & Continue      |
| [squarefunc.y](examples/squarefunc.y) | Basic function call   | 
| [chaincall.y](examples/chaincall.y)   | Chained function call |
| [perfect.y](examples/perfect.y)       | Perfect number check  |
| [times.y](examples/times.y)           | Multiplication table  |

## Quick Example
```y
x = 10;
y = 20;
println("sum = " + (x + y));

for(i=1; i<6; i+=1) {
    if(i == 3) {
        println("halfway!");
        continue;
    }
    println(i);
}

fn square(n) {
    return n * n;
}

println("square(5) = " + square(5));
```
Output:
```
sum = 30
1
2
halfway!
4
5
square(5) = 25
```



---

## Tech Stack

* **Language:** *C++20*
* **Build System:** CMake
* **Runtime:** Custom Stack-Based Virtual Machine
* **Platform:** Cross-platform



## Future Work

* [ ] REPL
* [ ] Recursion
* [ ] List / Map
* [ ] Formatted strings
* [ ] Class
* [ ] Automated tests
* [ ] Documentation
* ...and more



## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.


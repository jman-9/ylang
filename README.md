# ylang 0.0.1 — *it runs*

A lightweight programming language featuring a complete compilation pipeline from source to bytecode execution.

---

## Overview

`ylang` is a minimal scripting language project designed to prove the full flow of a working language system:
from lexical analysis to execution on a custom virtual machine.



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
* control statements
  * if / for / break / continue / return
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
1. Open the project folder in **Visual Studio 2022**
2. Select configuration
3. Build
4. Run
```

#### Using CLI

```cmd
cmake --preset x64-debug
cmake --build --preset x64-debug
### Windows GUI (MSVC)
```

### Linux / macOS (planned verification)

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

---

## Examples

| File            | Description           |
| --------------- | --------------------- |
| `hello.y`       | Basic print           |
| `cond.y`        | Conditional test      |
| `forloop.y`     | Simple loop & sum     |
| `forctrl.y`     | Break & Continue      |
| `squarefunc.y`  | Basic function call   | 
| `chaincall.y`   | Chained function call |
| `perfect.y`     | Perfect number check  |
| `times.y`       | Multiplication table  |

---

## Tech Stack

* **Language:** *C++20*
* **Build System:** CMake
* **Runtime:** Custom Stack-Based Virtual Machine
* **Platform:** not dependent



## Future Work

* [ ] REPL
* [ ] Recursion
* [ ] Array / Map
* [ ] Formatted strings
* [ ] Class
* [ ] Automated tests
* [ ] Documentation
* ...and more



## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.


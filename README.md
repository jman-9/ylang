# ylang 0.0.2

A programming language — retro, C-like and Pythonic

---

## Overview

`ylang` is a lightweight, C-like language designed with a simple and retro style —
combining C’s syntax with Python-like dynamic semantics.


It implements a complete compilation pipeline — from scanning to bytecode generation — and execution on its own virtual machine, named `yvm` — keeping it small, structured, and clear.

It now supports REPL, dynamic resolution via dot(`.`) operator,
and core collections (list, dict).


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

* Dynamic variables (number, string, list, dictionary)
* Control statements: if/for/break/continue/return
* User functions and recursion
* dot(`.`) operator for dynamic member resolution
* Built-in methods for list, dict, and string
* Stack-based virtual machine execution


## Project Structure

```
ylang/
├─ core/                 # ylang core - Scanner, Parser, Semantic, Bytecode, etc
│   └─ vm/               # yvm - Machine 
├─ examples/             # Example scripts (.y)
├─ tests/                # test programs
└─ tools/                # tools
    └─ ylang/            # ylang CLI


```



## Build Instructions

### Prerequisites

* CMake ≥ 3.26
* C++20 compatible compiler
* Visual Studio 2022 (Windows) or GCC/Clang (Linux, macOS)

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

### Script Mode

Run with source file:

```bash
λ ylang examples/07_string.y
```

Example result:

```
=== string example ===
11
6
hello
hello ylang
['a', 'b', 'c', 'd']
['apple', 'banana', '', 'grape']
```

### Interactive Mode (REPL)

`ylang` provides an interactive REPL (Read–Eval–Print Loop)
that supports multi-line functions, global state, 
and runs code interactively.

```text
λ ylang
ylang 0.0.2

>> fn avg(list) {
..     sum = 0;
..     for (x = 0; x < list.len(); x += 1) {
..         sum += list[x];
..     }
..     return sum / list.len();
.. }

>> nums = [3, 6, 10, 5];
>> println("average = {avg(nums)}");
average = 6

>> fn greet(person) {
..     return "hello " + person['name'] + ", age " + person['age'];
.. }

>> alice = { 'name': 'Alice', 'age': 30 };
>> println(greet(alice));
hello Alice, age 30
```



## Examples

| File                                      | Description                     |
| ------------------------------------------| ------------------------------- |
| [01_control.y](examples/01_control.y)     | if / for / break / continue     |
| [02_function.y](examples/02_function.y)   | user function and recursion     |
| [03_fibonacci.y](examples/03_fibonacci.y) | recursive fibonacci             |
| [04_list.y](examples/04_list.y)           | list example                    |
| [05_dict.y](examples/05_dict.y)           | dictionary example              | 
| [06_list_dict.y](examples/06_list_dict.y) | list-dict combined example      |
| [07_string.y](examples/07_string.y)       | string example                  |
| [08_perfect.y](examples/08_perfect.y)     | perfect number calculation demo |

## Quick Example
```rust
println("=== quick example ===");

fn add(x, y) {
    return x + y;
}

msg = "hello ylang";
words = msg.split();
println(words);                    // ['hello', 'ylang']

nums = [10, 20, 30];
nums.append(add(40, 2));           // use user-defined function
println("list: {nums}");           // [10, 20, 30, 42]

fn main() {
    user = {'name': "alice", 'age': 25, 'scores': nums};
    println(user);                 // {'name': 'alice', 'age': 25, 'scores': [10, 20, 30, 42]}
    println(user.keys());          // ['name', 'age', 'scores']
    println(user['scores'].len()); // 4

    msg2 = msg.replace("ylang", "world");
    println(msg2);                 // hello world
}
```
Output:
```bash
=== quick example ===
['hello', 'ylang']
list: [10, 20, 30, 42]
{'name': 'alice', 'age': 25, 'scores': [10, 20, 30, 42]}
['name', 'age', 'scores']
4
hello world
```



---

## Tech Stack

* **Language:** *C++20*
* **Build System:** CMake
* **Runtime:** Stack-based VM (yvm)
* **Typing:** Dynamic
* **Platform:** Cross-platform



## Future Work

* [x] REPL
* [x] Recursion
* [x] List / Dict
* [x] Floating-point
* [x] f-string
* [x] main() entrypoint
* [ ] String formatting
* [ ] Class
* [ ] Memory management
* [ ] Module & import system
* [ ] Optimized VM dispatch
* [ ] Automated tests
* [ ] Documentation
* ...and more



## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.



## Version History

| Version    | Highlights                                                            |
| ---------- | --------------------------------------------------------------------- |
| **0.0.1** | Basic VM, arithmetic, control flow, user functions                    |
| **0.0.2** | Dynamic resolution, collections(list/dict), f-string, floating-point, main() entrypoint |

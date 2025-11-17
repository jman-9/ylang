# ylang 0.0.3

yet another programming language — retro, C-like, and Pythonic

---

## Overview

`ylang` is a lightweight, C-like language designed with a simple and retro style —
combining C’s syntax with Python-like dynamic semantics.


It implements a complete compilation pipeline — from scanning to bytecode generation — and execution on its own virtual machine, named `yvm` — keeping it small, structured, and clear.


## Language Characteristics

### C-like
* C-style syntax
* `main` function as the entry point
* Semicolon(`;`) required for statement termination
* Control statements: if/for/break/continue/return
* Same operators as C (arithmetic, logical, bitwise, augmented assignments)

### Pythonic
* Supports both top-level statements and `main()` entry function
* Dynamic typing (number, string, collection, object)
* Built-in collections: list, dictionary
* f-string style interpolation — `"value = {value}"` (but no prefix!)
* User-defined functions
* Dot(`.`) operator for dynamic member access
* Primitive modules: string, list, dictionary
* Useful built-in modules: json, math, file, rand, sys


## Architecture

**Project Structure**

```
ylang/
├─ core/              # ylang core - compiler engine
│   ├─ builtin/       # Core built-in modules
│   ├─ module/        # Module loader
│   ├─ vm/            # yvm engine
│   └─ primitives/    # Base types (list, dict, string)
├─ examples/          # Example scripts (.y)
├─ tests/             # test programs
│   └─ unittest/      # Unit tests (Catch2)
└─ tools/             # tools for ylang
    └─ ylang/         # ylang CLI
```

**Pipeline**

```
Source (.y)
 → Scanner
 → String Interpolator
 → Parser
 → Semantic Analyzer
 → Bytecode Builder
 → Machine (yvm)
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
λ ylang examples/basic/03_fibonacci.y
```

Example result: 

```
=== fibonacci example ===
0 1 1 2 3 5 8 13 21 34
```

### Interactive Mode (REPL)

`ylang` provides an interactive REPL (Read–Eval–Print Loop)
that supports multi-line functions, global state, 
and runs code interactively.

```text
λ ylang
ylang 0.0.3
>> include json;
>> d = 
..  json.parse('{{"a":1,"b":2}}');
..
>> println("{d['a']} + {d['b']} = " + (d['a'] + d['b']));
1 + 2 = 3
```



## Examples

#### See [basic examples](examples/basic/)

### More complex examples

[Langton's ant (ANSI visual animation)](examples/langton_ant.y) — (ANSI visual animation)

[Maze generation and A* pathfinding](examples/maze_gen_find.y) — (also animation :)

[CSV to JSON to CSV demo](examples/csv_json_csv.y)


## Quick Example
```rust
include json;

println("=== quick example ===");

fn main() {
    data = { "name": "Alice", "age": 25, "scores": [85, 90, 78] };
    json_text = json.dump(data);
	println(json_text);

    parsed = json.parse(json_text);
    println("name = {parsed['name']}, age = {parsed['age']}");
}
```

Output: 

```bash
=== quick example ===
{"age":25,"name":"Alice","scores":[85,90,78]}
name = Alice, age = 25
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
* [x] Module & import system
* [ ] Optimized VM dispatch
* [x] Automated tests
* [ ] Documentation
* [ ] Unicode
* [ ] Increment and decrement operators
* [ ] Optimization
* ...and more


## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.


## Version History

| Version    | Highlights                                                            |
| ---------- | --------------------------------------------------------------------- |
| **0.0.1** | Basic VM, arithmetic, control flow, user functions                    |
| **0.0.2** | Dynamic resolution, collections(list/dict), f-string, floating-point, main() entrypoint |
| **0.0.3** | Module system, built-in modules, escape character, `yvm` refactoring |

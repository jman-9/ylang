<img src="ylang_logo.png" height="100">

# ylang 0.1.1

yet another programming language — retro, C-like, and Pythonic

---

## Overview

`ylang` is a lightweight, C-like language designed with a simple and retro style —
combining C’s syntax with Python-like dynamic semantics.

And beyond these, `ylang` also has its own style.

It implements a complete compilation pipeline — from scanning to bytecode generation — and execution on its own virtual machine, named `yvm` — keeping it small, structured, and clear.

## What's New

* 0.1.1 introduces a stabilized architecture:
  - module include system with namespaces
  - restructured VM
    - redesigned scope system 
    - refined stack model
    - memory model based on reference counting

## Language Characteristics

### C-like
* C-style syntax
* `main` function as the entry point
* Semicolon(`;`) required for statement termination
* Control statements: if/for/break/continue/return
* Same operators as C (arithmetic, logical, bitwise, augmented assignments)
* Increment and decrement operators (`++`, `--`)

### Pythonic
* Supports both top-level statements and `main()` entry function
* Dynamic typing (number, string, collection, object)
* Built-in collections: list, dictionary
* f-string style interpolation — `"value = {value}"` (but no prefix!)
* User-defined functions
* Dot(`.`) operator for dynamic member access
* Primitive modules: string, list, dictionary
* Useful built-in modules: json, math, file, rand, sys

### My own style
* Supports lightweight class system
  - Classes with fields and member functions
  - Constructor defined with the class name (C++ style)
  - `this` or `self` keyword not required inside member functions
* Module include with automatic namespace extraction
* (more to come)

## Architecture

**Project Structure**

```
ylang/
├─ core/              # ylang core - compiler engine
│   ├─ builtin/       # Core built-in modules
│   ├─ compiler/      # compiler core
│   ├─ contract/      # ABI layer
│   ├─ module/        # Module loader
│   ├─ util/          # utility for core
│   ├─ vm/            # yvm engine
│   └─ primitives/    # Base types (list, dict, string)
├─ doc/               # Documents
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
* Visual Studio 2017+ (Windows) or GCC/Clang (Linux, macOS)

### Windows (Visual Studio 2017+)

#### Using GUI
```
1. Open the project folder in Visual Studio
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

## Module Include & Namespace Rules

ylang supports module include using dot paths, slash paths, relative paths, and absolute paths.
Namespaces are assigned automatically:

```rust
include util.math;          // namespace = util.math
include engine/renderer;    // namespace = renderer
include ../shared/logger;   // namespace = logger
include /abs/path/world.ai; // namespace = world.ai
```

Rules:

* `a.b.c` → namespace = `a.b.c`
* `a/b/c` → namespace = `c`
* relative paths allowed (`../x/y.z` → `y.z`)
* absolute paths allowed (`/p/q/r.s` → `r.s`)

Usage:

```rust
println(util.math.pi);
engine.renderer.draw();
logger.info("start");
world.ai.run();
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
ylang 0.0.5
>> include json;
>> d = 
..  json.parse('{{"a":1,"b":2}}');
..
>> println("{d['a']} + {d['b']} = " + (d['a'] + d['b']));
1 + 2 = 3
```



## Examples

**See: [basic examples](examples/basic/)**

### Complex examples

[Langton's ant](examples/langton_ant.y) — (ANSI visual animation)

[Maze generation and A* pathfinding](examples/maze_gen_find.y) — (also animation :)

[Text RPG adventure simulation](examples/rpg_sim.y)

[Grand strategy simulation](examples/grand_strategy/) — (large-scale module / namespace demo)

**➜ [More examples](examples/)**


## Quick Example
```rust
class Player {
    _name = "";
    _hp = 100;

    fn Player(name, hp) {
        _name = name;
		_hp = hp;
    }
    fn damage(amount) { _hp -= amount; }
    fn status() { println("{_name}: {_hp} HP"); }
}

println("=== quick example ===");
include json;

fn main() {
    data = { "name": "Alice", "hp": 100 };
    json_text = json.dump(data);
	println("dumped JSON text: " + json_text);	
	
    parsed = json.parse(json_text);
    println("parsed JSON object: {parsed}");
	println();
	
	println("--- class behavior ---");
	damage = 25;	
	p = Player(parsed['name'], parsed['hp']);
	p.status();
	p.damage(damage);
	println('{p._name} takes {damage} damage.');
	p.status();	
}

```

Output: 

```bash
=== quick example ===
dumped JSON text: {"hp":100,"name":"Alice"}
parsed JSON object: {'hp': 100, 'name': 'Alice'}

--- class behavior ---
Alice: 100 HP
Alice takes 25 damage.
Alice: 75 HP
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
* [x] Class
* [x] Memory management (refcnt, dynamic resizing)
* [x] Module & import system
* [x] Optimized VM dispatch
* [x] Automated tests
* [ ] Documentation
* [ ] Unicode
* [x] Increment and decrement operators
* [x] Source code import system
* [ ] Exception
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
| **0.0.5** | Class system, increment/decrement operators (`++`,`--`), chained assignments (`a=b=0`), basic literals (`null`,`true`,`false`)|
| **0.1.0** | Namespace rule, source-based include, VM restructuring (scope/stack redesign, refcnt memory model, dynamic resizing)|
| **0.1.1** | Fixed critical bugs, Refactored code, class field declaration using chained assignments, backward declarations in classes |

---

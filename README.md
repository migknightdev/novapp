# Nova++ Toolchain (`vcomp`) 🚀

A lightweight, custom programming language transpiler and compiler toolchain written in **C++17**. `vcomp` parses Nova++ source files (`.nvpp`), automatically injects standard boilerplate, manages lazy type deduction, processes custom scopes, and invokes `g++` to generate high-performance native binaries.

Current Version: **0.2.0-beta**

---

## ⚡ Features

* **Procedural Functions:** Declare reusable blocks of code using the `func` keyword, separating global definition logic from the execution scope effortlessly.
* **Boilerplate Automation:** Automatically injects the required standard C++ headers and variadic template utilities under the hood.
* **`noMain` Directive:** Write clean, quick-start scripts without manually declaring entry points or handling runtime configurations.
* **Lazy Type Inference:** Automatic type deduction (`auto`) for variables, numbers, booleans, and strings without manual type declarations.
* **Dynamic Input Handling:** Natively captures user console inputs via the `input` keyword mapped directly to existing static variables.
* **Conditional Branching:** Full support for logical decision structures using `if`, `elif` (translated to `else if`), and `else` blocks with native curly braces `{}`.
* **Native UTF-8 Encoding:** Full run-time support for special characters and accents (e.g., `ç`, `ã`, `é`) directly enforced in the Windows console environment.
* **Automatic Workspace Cleanup:** Transpiles, compiles, and automatically deletes the intermediate `.cpp` file, keeping your working directories clean.
* **Exact Error Tracking:** Aborts safely and tells you the exact line number and code statement where a syntax violation occurred before downstream compilation.
* **Compiler-Grade Logs:** Professional, clean, and standardized English terminal logs.

---

## 🛠️ Installation & Setup

### Prerequisites
Make sure you have GCC/G++ installed and configured in your system's environment variables (`PATH`).

### Option 1: Downloading Pre-built Binaries (Recommended)
You can skip manual compilation by downloading the latest stable executable directly from the official repository:
1. Access the [Nova++ Releases](https://github.com/migknightdev/novapp/releases) page.
2. Download the latest `novapp_setup.exe` binary.
3. execute the setup

### Option 2: Building from Source
To compile the `vcomp` executable yourself from the source code, run the following command in your terminal:

```bash
g++ -std=c++17 vcomp.cpp -o vcomp
```
💻 Usage
Command Line Arguments
Check Compiler Version:

```Bash
vcomp --version
```
Or using the short flag:
```Bash
vcomp -v
```
Compile a Nova++ Source File:
```Bash
vcomp file.nvpp
```
📝 Syntax Example
Create a file named game.nvpp with the following code to test functions, inputs, and conditional branches:
```nvpp
#include <nova>
using noMain

func jogo() {
    pf "welcome to calculation game"
    pf "how is 5 - 3?"
    
    trueAnswer = 2
    answer = 0
    input answer

    if (answer == trueAnswer) {
        pf "you win \n"
    } elif (answer == 54) {
        pf "no \n"
    } else {
        pf "you lost \n"
    }
}

// Executing the custom function
jogo()

exit 0
```
Compiling and Running:
Run the compiler over your file:

```Bash
vcomp game.nvpp
```
Under the Hood:
The toolchain will automatically isolate your func blocks into the global C++ scope, transpile the procedural statements into standard C++ (game.cpp), inject the UTF-8 locale configuration, trigger GCC to output a standalone native executable binary (game.exe), and safely erase the intermediate game.cpp source file.

🛑 Error Handling Example
If you forget to provide arguments to a function, make a mistake in a variable declaration, or misconfigure the input targeting, vcomp will intercept the issue and report the location accurately:
``` Plaintext
[SYNTAX ERROR] Line 8: 'input' requires a target variable.
 -> Statement: "input "
 ```
 [![Syscore](https://img.shields.io/badge/MADE_BY-SYSCORE-black?style=flat-square&logo=visual-studio-code&logoColor=blue)](https://github.com/migknightdev)

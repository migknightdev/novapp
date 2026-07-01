# Nova++ Toolchain (`vcomp`) 🚀

A lightweight, custom programming language transpiler and compiler toolchain written in **C++17**. `vcomp` parses Nova++ source files (`.nvpp`), automatically injects standard boilerplate, manages lazy type deduction, and invokes `g++` to generate high-performance native binaries.

Current Version: **0.1.9-alpha**

---

## ⚡ Features

* **Boilerplate Automation:** Automatically injects the required standard C++ headers and components under the hood.
* **`noMain` Directive:** Write clean, quick-start scripts without manually declaring entry points.
* **Lazy Type Inference:** Automatic type deduction (`auto`) for variables, numbers, booleans, and strings without manual type declarations.
* **Dynamic Input Handling:** Natively captures user console inputs via the `input` keyword mapped directly to existing static variables.
* **Conditional Branching:** Support for logical decision structures using `if`, `elif`, and `else` blocks with native curly braces `{}`.
* **Native UTF-8 Encoding:** Full run-time support for special characters and accents (e.g., `ç`, `ã`, `é`) directly in the Windows console.
* **Automatic Workspace Cleanup:** Transpiles, compiles, and automatically deletes the intermediate `.cpp` file, keeping your directories clean.
* **Exact Error Tracking:** Tells you the exact line and code statement where a syntax violation occurred.
* **English Logs:** Professional, compiler-grade terminal logs.

---

## 🛠️ Installation & Setup

### Prerequisites
Make sure you have GCC/G++ installed and configured in your system's `PATH`.

### 1. Building the Compiler
To compile the `vcomp` executable from the source code, run the following command in your terminal:

```bash
g++ -std=c++17 vcomp.cpp -o vcomp
```

### 2. Global Environment Setup (Optional)
Move the generated `vcomp.exe` to a permanent directory (e.g., `C:\compiler\`) and add that directory to your system's Environment Variables (`PATH`) to access the `vcomp` command globally from any folder.

---

## 💻 Usage

### Command Line Arguments

**Check Version:**
```bash
vcomp --version
```
Or using the short flag:
```bash
vcomp -v
```

**Compile a Nova++ File:**
```bash
vcomp file.nvpp
```

---

## 📝 Syntax Example

Create a file named `game.nvpp` with the following code to test the new features:

```nvpp
#include <nova>
using noMain

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

exit 0
```

### Compiling and Running:
Run `vcomp game.nvpp`.

The toolchain will automatically transpile it into standard C++ (`game.cpp`), inject the UTF-8 locale configuration, trigger GCC to output a standalone native executable (`game.exe`), and safely erase the intermediate `game.cpp` source file.

---

## 🛑 Error Handling Example

If you forget to provide arguments to a function, make a mistake in a variable declaration, or misconfigure the `input` targeting, `vcomp` will abort safely and report the exact location before downstream compilation:

```plaintext
[SYNTAX ERROR] Line 8: 'input' requires a target variable.
 -> Statement: "input "
```

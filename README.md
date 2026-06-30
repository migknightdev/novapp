# Nova++ Toolchain (`vcomp`) 🚀

A lightweight, custom programming language transpiler and compiler toolchain written in **C++17**. `vcomp` parses Nova++ source files (`.nvpp`), automatically injects standard boilerplate, manages simple type deduction, and invokes `g++` to generate high-performance native binaries.

Current Version: **0.1.8-alpha**

---

## ⚡ Features

* **Boilerplate Automation:** Automatically injects the required standard C++ headers and components under the hood.
* **`noMain` Directive:** Write clean, quick-start scripts without manually declaring entry points.
* **Type Inference:** Automatic type deduction (`auto`) for basic variables, numbers, booleans, and strings.
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

Create a file named `test.nvpp` with the following code:

```nvpp
#include <nova>
using noMain

pf "hello, world!"
math = 3 + 5
pf math
```

### Compiling and Running:
Run `vcomp test.nvpp`.

The toolchain will automatically transpile it into standard C++ (`test.cpp`) and trigger GCC to output a fast, standalone native executable (`test.exe`).

---

## 🛑 Error Handling Example

If you forget to provide arguments to a print function or make a mistake in a variable declaration, `vcomp` will abort safely and report the exact location:

```plaintext
[SYNTAX ERROR] Line 4: 'pf' function requires arguments.
 -> Statement: "pf "
```

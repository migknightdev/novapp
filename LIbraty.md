# 📚 Standard Library & Syntax Specification: Nova++ (v0.1.9-alpha)

Welcome to the official documentation for the Nova++ programming language. The vcomp compiler operates as a high-performance transpiler that converts .nvpp source files into native standard C++17, automatically handling compilation downstream to generate fast executable binaries.

---

## 🚀 Initialization Directives

Every Nova++ application utilizing the streamlined script layout must declare the core boilerplate header and entry point directive at the absolute top of the source file.

| Directive | Description | C++ Equivalent Under the Hood |
| :--- | :--- | :--- |
| #include <nova> | Ingests essential macros and foundational standard components. | Injects <iostream>, <string>, <cstdlib> |
| using noMain | Bypasses the requirement of writing manual main function scopes. | int main() { ... } |

---

## 🖥️ Streamlined I/O (Input / Output)

### pf (Print Function)
Outputs data directly to the terminal standard output. It natively supports string literals, evaluated variables, arithmetic expressions, and seamless inline concatenation via the + operator. Appends a trailing newline automatically.

* Syntax: pf <content>
* Example:
  pf "Hello World!"
  pf "The result is: " + (3 + 5)

### input
Captures user terminal input and maps it safely into an existing variable. Rule: The target variable must be initialized prior to calling input to ensure strict static type deduction at compile time.

* Syntax: input <target_variable>
* Example:
  username = ""
  input username

---

## 📦 Variable Allocation & Lazy-Type Deduction

Nova++ features automatic lazy type inference. You do not explicitly declare primitive types such as int, string, or bool. The vcomp engine parses the assignment payload to the right of the = operator and enforces correct static C++ allocations behind the scenes.

* String Literal: name = "SysKore"
* Integer Number: score = 100
* Boolean Flag: isActive = true

> ⚠️ Syntax Warning: Never pass native C++ datatype prefixes (e.g., int x = 0;). Simply write x = 0. The double equal sign == is treated strictly as an evaluation operator and does not trigger variable initialization routines.

---

## 🔀 Conditional Branching Structures

Conditional blocks direct the processing flow based on logical boolean evaluations. The parser uses standard curly braces {} to define localized scopes.

| Nova++ Structure | C++ Translation | Execution Scope |
| :--- | :--- | :--- |
| if (condition) { | if (condition) { | Executes only if the condition evaluates to true. |
| elif (condition) { | else if (condition) { | Evaluates if preceding conditions fail and current condition is true. |
| else { | else { | Catch-all block; executes if all preceding evaluations fail. |
| } | } | Terminates the current active conditional evaluation block. |

### Complete Flow Example:
#include <nova>
using noMain

pf "Enter your current age:"
userAge = 0
input userAge

if (userAge >= 18) {
    pf "Access granted."
} elif (userAge == 17) {
    pf "Access denied. One more year to go!"
} else {
    pf "Access strictly denied."
}

---

## ⚙️ Process & Flow Control

### exit
Immediately terminates program execution and passes an exit status code back to the underlying operating system. The compiler automatically hooks a system native pause routine (system("pause")) prior to termination to prevent the terminal window from closing unexpectedly.

* Syntax: exit <return_code>
* Example:
  exit 0 // Successful program execution termination

---

## 🛠️ Command-Line Interface (CLI) Compilation

To compile a Nova++ program, invoke the custom vcomp executable binary from your command line interface, passing your target source script file as an argument:

# Transpiles and compiles your source script down to a native binary
vcomp game.nvpp

# Outputs current installed compiler environment metadata
vcomp --version
# 🧠 AI-Integrated Custom Shell (Gshell)

A **custom Unix shell** implemented in **C**, enhanced with a built-in **AI command assistant**.  
This shell supports core Unix shell functionalities — pipelines, redirections, job control — along with an intelligent layer that interprets **natural language commands** and **learns from context** such as recent commands, errors, and system information.

---

## ⚙️ Features

### 🧩 Core Shell Capabilities
1. **Command Execution** — Run system commands directly (`ls`, `cat`, etc.)
2. **Pipelines** — Connect multiple commands (`ls | grep foo`)
3. **Command Chaining** — Conditional execution (`cmd1 && cmd2`, `cmd1 || cmd2`, `cmd1; cmd2`)
4. **I/O Redirection** — Input/output redirection (`<`, `>`, `>>`)
5. **Background Jobs** — Execute tasks asynchronously (`sleep 10 &`)
6. **Job Control** — Manage process groups and foreground/background processes
7. **Signal Handling** — Handles `SIGINT`, `SIGTSTP`, `SIGCHLD` gracefully
8. **Terminal Control** — Uses `termios` for raw mode input (custom line editing, Ctrl+C, Backspace, etc.)

---

### 🤖 AI Integration Layer

The **AI system** extends shell functionality by allowing you to interact using natural language commands that start with `ai`.

#### 🧠 AI Capabilities
- **Understand Intent:** Interprets commands like  
  - `ai explain about loop in shell scripting`  
  - `ai create a new directory in parent directory of where i am`
- **Context Awareness:** Considers  
  - The **past 5 commands**
  - **OS information** (like working directory, environment)
  - **Recent errors** or failed command logs
- **Self-Learning Error Recovery:**  
  - When a command fails, the error is saved temporarily  
  - The AI uses this context to **refine and re-run** the command automatically
- **Guardrails & Safety:**  
  - Warns users before executing potentially unsafe commands  
  - Explains what each command will do before running it  

#### 🧾 Example AI Commands
```bash
ai explain about environment variables
ai show me what went wrong with my last command
ai create folder named temp in parent directory
ai list processes and explain which one is using most CPU
````

---

## 🖥️ Installation & Execution

### 1. Prerequisites

Ensure your system has:

* A **Linux/Unix** environment
* `gcc` compiler
* `make` utility

### 2. Setup

Clone the repository and compile:

```bash
git clone https://github.com/gaurav-k-prasad/shell.git
cd gshell
make
```

### 3. Run the Shell

```bash
./gshell
```

---

## 🧱 Example Commands

```bash
# Standard commands
ls -l | grep .c
make && ./a.out
sleep 5 &

# Mixed complex command
ls | sort && echo "done" & cat < read.txt > write.txt

# AI-driven commands
ai explain about fork in linux
ai create a new directory in the parent directory of current folder
```

---

## 🧠 System Overview

| Module               | Description                                                                                                     |
| -------------------- | --------------------------------------------------------------------------------------------------------------- |
| **Parser**           | Tokenizes input and builds command structures                                                                   |
| **Executor**         | Executes commands using `fork()`, `execvp()`, and handles pipes/redirection                                     |
| **Job Control**      | Manages background/foreground processes using signals                                                           |
| **Terminal Handler** | Uses `termios` for raw input, handling keys and signals                                                         |
| **AI Layer**         | Parses natural language, uses context (recent commands, OS info, errors), and generates safe executable actions |
| **Error Logger**     | Records failed executions and provides context for the AI system to learn from                                  |

---

## 🧩 Design Highlights

* **Process Management:** Built using `fork()`, `execvp()`, `waitpid()`, and `pipe()`
* **Terminal Handling:** Implemented using raw input via `termios`
* **Signal Safety:** Handles `SIGINT`, `SIGTSTP`, `SIGCHLD` gracefully
* **Contextual AI:** Analyzes previous interactions and command failures
* **Security Guardrails:** Prevents harmful commands and provides warnings before execution

---

## 📚 Author’s Note

This project is developed purely in **C** for **Linux/Unix-based systems** and is not compatible with Windows.
It serves as both an **educational tool** for learning Unix process management and a **demonstration** of AI-enhanced command-line automation.

The AI integration showcases how natural language processing concepts can make system-level interfaces more accessible, adaptive, and safe to use.

---

## 🧾 License

This project is open source under the **MIT License**.

---

## 🙌 Acknowledgements

* *The Linux man pages* (`man 2 fork`, `man 3 termios`, etc.)
* *Advanced Programming in the UNIX Environment* by W. Richard Stevens
* POSIX documentation for process and signal management
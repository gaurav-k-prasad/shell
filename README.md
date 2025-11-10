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
```markdown
# Gshell — A Simple Unix-like Shell (C)

Gshell is a compact Unix-like command shell implemented in C. It provides a small, well-scoped set of POSIX-compatible shell features intended for learning, experimentation, and embedding in constrained environments.

This repository contains the source code, documentation, and utilities needed to build and run the shell on Linux/Unix systems.

## Features

- Command execution with arguments and environment
- Pipelines (`cmd1 | cmd2`)
- Conditional and sequential execution (`&&`, `||`, `;`)
- I/O redirection (`<`, `>`, `>>`)
- Background execution (`&`) and basic job control
- Builtin commands (e.g. `cd`, `pwd`, `echo`, `env`, `export`, `unset`, `which`, `source`)
- Custom line-editing and history (in-memory, fixed capacity)
- Signal handling and terminal control using `termios`

Note: The repository includes an optional command that integrates with an external helper process to produce suggested shell commands. That integration is not required for normal shell operation and is not emphasized in the documentation.

## Documentation

Detailed, topic-specific documentation is provided in the `docs/` directory. See:

- `docs/features.md` — full feature list and behavior
- `docs/architecture.md` — design notes and module responsibilities
- `docs/usage.md` — build and runtime instructions
- `docs/todos.md` — planned work and improvements
- `docs/bugs.md` — known issues and edge cases

## Build & Run

Prerequisites:

- Linux/Unix environment
- `gcc` (or compatible C compiler)
- `make`

Build:

```sh
make
```

Run:

```sh
./gshell
```

## Contributing

Contributions are welcome. When submitting changes, prefer small, well-scoped pull requests. Suggested areas for improvement include tests, improved input handling, and safer temporary-file handling.

If you add features that change the user-visible behavior, update the relevant `docs/*.md` files.

## License

This project is licensed under the MIT License. See `LICENSE` for details when present.

```
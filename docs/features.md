# Features

This document lists the functional capabilities implemented by Gshell.

Core shell capabilities

- Command execution with arguments and explicit environment
- Pipelines using `|`
- Command chaining and conditional execution using `&&`, `||`, and `;`
- I/O redirection using `<`, `>`, and `>>`
- Background execution (`&`) and basic job control (process groups)
- Signal handling (SIGINT, SIGCHLD, SIGWINCH) for robust interactive use
- Terminal control with `termios` for raw-mode input and basic line-editing
- In-memory command history implemented as a fixed-capacity doubly-linked list
- Builtin commands executed in-process when required: `cd`, `pwd`, `echo`, `env`, `export`, `unset`, `source`, `which`
- A small set of helper builtins executable in forked children: `echo`, `pwd`, `env`, `which`

Notes

- The shell avoids heavy external dependencies; features are implemented in plain C and rely on POSIX APIs.
- The repository also contains an optional integration that communicates with an external helper for generating suggested commands. This is optional and not required for normal shell usage.

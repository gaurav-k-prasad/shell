# Architecture

This document summarizes the high-level architecture and responsibilities of the major modules.

Overview

The shell provides a small, modular codebase where each principal concern is encapsulated in a source file:

- `main.c` — Process lifecycle and REPL loop. Initializes terminal state, registers signal handlers, and drives the interactive loop.
- `input.c` — Terminal raw mode and line-editing features (cursor movement, insert/delete, history navigation).
- `helpers.c` — Utility functions: prompt rendering, token and vector constructors/destructors, environment helpers, and small I/O utilities.
- `parser.c` — Lexical analysis and command-structure construction. Produces a hierarchy of tokens, pipeline components, pipelines, and commands.
- `executor.c` — Command execution with pipes, redirections, forking, process groups, and job control.
- `builtins.c` — Implementations of shell builtins (cd, export, unset, source, which, etc.) and orchestration helpers for optional external integrations.
- `aiParser.c` — Parser for structured output produced by the external helper (kept separate from core shell logic).
- `forgettingDLL.c` — Fixed-capacity doubly-linked list used for in-memory history storage.

Design notes

- Processes and job control: the executor creates process groups and uses `tcsetpgrp` to provide terminal control to foreground jobs. Background jobs are always placed in their own process group.
- Builtins that change the shell state (e.g. `cd`, `export`, `unset`, `source`, `exit`) are executed in the main process when required. Other builtins may be executed in forked children as needed.
- Terminal input is implemented directly using `termios` and raw mode. This keeps the dependency footprint minimal and the codebase educationally valuable.
- File-based communication with external helpers is used for integration simplicity. Consider migrating to `mkstemp` or unique temporary files if multiple concurrent shell instances must be supported.

Error handling and memory safety

- The code uses explicit allocation and freeing patterns for dynamic structures. Error paths attempt to free partially-constructed objects. Some complex error paths may still benefit from audit and tests.


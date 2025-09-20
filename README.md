# Shell in C Programming Language

This shell can only be executed on linux/unix based systems

To run the shell -

1. Install `make` on your system
2. put `make` on terminal
3. run `./gshell` to run the shell


## Custom Shell with Job Control and Terminal Handling
### Overview

* This project implements a minimal shell in C with support for:

1. Pipelines (ls | grep foo)

2. Command chaining (cmd1 && cmd2, cmd1 || cmd2)

3. Background jobs (cmd &)

4. Process groups and terminal control

5. Raw mode input (custom line editing / interactive features)

6. It also demonstrates correct interaction with the terminal driver (termios) and job control signals (SIGINT, SIGTSTP, SIGCHLD).

* Terminal Modes

  The shell runs in raw mode only when it is reading input, to allow fine-grained control over:

1. character-by-character input

2. intercepting Ctrl+C, Ctrl+D, arrow keys, Ctrl + Backspace, Backspace etc.

---

### Example Supported Commands

* foreground pipeline `ls | grep main.c`

* Conditional execution
  `make && ./a.out`

* background jobs
  `sleep 10 &`

## Mixed Commands
* Complecated commands like `ls | sort && echo "done" &`
* ```wc -l < read1.txt < read2.txt > write1.txt < read3.txt && echo $PATH:$LANG || echo '\n' & ls; cat < read.txt > write.txt```

---

### Author's Note
* The code utilizes fork(), execve() which are only available in linux based systems

* Not implemented yet for windows based system

* Thank you

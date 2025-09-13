# Shell Features

## 1. Shell Loop

- Main loop to keep the shell running

## 2. Input Parsing

- Read and parse user commands

## 3. Command Execution (Built-in Commands)

### Built-in Commands

1. `cd` – Change directory  
2. `pwd` – Print working directory  
3. `echo` – Display text  
4. `env` – Print environment variables  
5. `setenv` – Set an environment variable  
6. `unsetenv` – Unset an environment variable  
7. `which` – Locate a command  
8. `exit` – Exit the shell  

## 4. Execute External Commands

- Run programs available in the system PATH

## 5. Manage Environment Variables

- Add, update, and remove variables

## 6. Manage Path

- Add/remove directories from the PATH

## 7. Error Handling

- Handle invalid commands, syntax errors, and runtime errors

- Not freeing memory
- signal processing ^C
- Error handling
- remove deprecated functions and their dependencies fix
  1. int shellBuilts(char ***args, char **env, char *initialDirectory);
  2. int executor(char ***args, char **env);
#include "myshell.h"

void shellLoop(char **env);
int shellBuilts(char **args, char **env, char *initialDirectory);

int main(int argc, char const *argv[], char *env[])
{
  (void)argc;
  (void)argv;
  
  shellLoop(env);
  return 0;
}

void shellLoop(char **env)
{
  char *input = NULL;
  size_t input_size = 0;
  char **args;
  char *initialDirectory = getcwd(NULL, 0);
  char buff[1024];

  while (1)
  {
    getcwd(buff, sizeof(buff));
    fprintf(stdout, "\033[35m[%s]> \033[0m", buff); // ANSI format coloring

    if (getline(&input, &input_size, stdin) == -1) // end of file -> (ctrl + z -> Enter)
    {
      perror("End fo file detected");
      break;
    }

    args = parseInput(input);

    if (args[0]) // if no arguments
    {
      shellBuilts(args, env, initialDirectory);
    }

    freeTokens(args);
  }
}

/*
 $ builtins - 
 *  1. cd
 *  2. pwd
 *  3. echo
 *  4. env
 *  5. setenv
 *  6. unsetenv
 *  7. which
 *  8. exit
  
 $ executor binary - 
 * ls
 * cat ...
 */
int shellBuilts(char **args, char **env, char *initialDirectory)
{
  if (myStrcmp(args[0], "cd") == 0)
  {
    return commandCd(args, initialDirectory);
  }
  else if (myStrcmp(args[0], "pwd") == 0)
  {
    return commandPwd();
  }
  else if (myStrcmp(args[0], "echo") == 0)
  {
    return commandEcho(args, env);
  }
  else if (myStrcmp(args[0], "env") == 0)
  {
    commandEnv(env);
  }
  else if (myStrcmp(args[0], "setenv") == 0)
  {
    // commandSetenv(args, env);
  }
  else if (myStrcmp(args[0], "unsetenv") == 0)
  {
    // commandUnsetenv(args, env);
  }
  else if (myStrcmp(args[0], "which") == 0)
  {
    commandWhich(args, env);
  }
  else if (myStrcmp(args[0], "exit") == 0 || myStrcmp(args[0], "quit") == 0)
  {
    exit(EXIT_SUCCESS);
  }
  else
  {
    puts("Command not found");
    // TODO executor();
  }
}
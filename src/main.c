#include "../headers/myshell.h"

void shellLoop(char **env);

int main(int argc, char const *argv[], char *env[])
{
  (void)argc;
  (void)argv;

  shellLoop(env);
  return 0;
}

void shellLoop(char **envp)
{
  char *input = NULL;
  size_t input_size = 0;
  // char **args;
  char ***args;
  char *initialDirectory = getcwd(NULL, 0);
  char buff[1024];
  char **env = cloneEnv(envp);

  if (!env)
  {
    fprintf(stderr, "enviornment duplication failed");
    exit(EXIT_FAILURE);
  }
  char *userName = myGetenv("LOGNAME", env);

  while (1)
  {
    getcwd(buff, sizeof(buff));
    if (userName)
      fprintf(stdout, "\033[36m@%s\033[0m \033[32m[%s]> \033[0m", userName, buff); // ANSI format coloring
    else
      fprintf(stdout, "\033[32m[%s]> \033[0m", buff); // ANSI format coloring

    if (getline(&input, &input_size, stdin) == -1) // end of file -> (ctrl + z -> Enter)
    {
      perror("End fo file detected");
      break;
    }
    input[strcspn(input, "\n")] = '\0';

    args = extendedParser(input);
    if (!args)
    {
      fprintf(stderr, "parsing failed");
      continue;
    }

    char **firstCommand = args[0];
    if (firstCommand) // if no arguments
    {
      if (myStrcmp(firstCommand[0], "export") == 0)
      {
        env = commandExport(firstCommand, env);
      }
      else if (myStrcmp(firstCommand[0], "unset") == 0)
      {
        env = commandUnset(firstCommand, env);
      }
      else
      {
        shellBuilts(args, env, initialDirectory);
      }
    }

    freeTokens(args);
  }

  free(userName);
  freeEnv(env);
}

/*
 $ builtins -
 *  1. cd
 *  2. pwd
 *  3. echo
 *  4. env
 *  5. export
 *  6. unset
 *  7. which
 *  8. exit

 $ executor binary -
 * ls
 * cat ...
 */
int shellBuilts(char ***args, char **env, char *initialDirectory)
{
  char **firstCommand = args[0];
  if (myStrcmp(firstCommand[0], "cd") == 0)
  {
    return commandCd(firstCommand, initialDirectory);
  }
  else if (myStrcmp(firstCommand[0], "pwd") == 0)
  {
    return commandPwd();
  }
  else if (myStrcmp(firstCommand[0], "echo") == 0)
  {
    return commandEcho(firstCommand, env);
  }
  else if (myStrcmp(firstCommand[0], "env") == 0)
  {
    return commandEnv(env);
  }
  else if (myStrcmp(firstCommand[0], "which") == 0)
  {
    return commandWhich(firstCommand, env);
  }
  else if (myStrcmp(firstCommand[0], "exit") == 0 || myStrcmp(firstCommand[0], "quit") == 0)
  {
    exit(EXIT_SUCCESS);
  }
  else
  {
    return executor(args, env);
  }
}
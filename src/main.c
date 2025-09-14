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
  // char ***args;
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

    VectorToken *tokenVec = getTokens(input, env);
    if (tokenVec == NULL)
    {
      fprintf(stdout, "Lexical phase failed");
      continue;
    }

    Commands *allCommands = splitCommands(tokenVec);
    if (allCommands == NULL)
    {
      freeVecToken(tokenVec);
      fprintf(stdout, "Parsing failed");
      continue;
    }

    freeVecToken(tokenVec);

    for (int i = 0; i < allCommands->commands->size; i++)
    {
      Command *command = allCommands->commands->data[i];
      int status = executeCommand(command, &env, initialDirectory);
      printf("---\nStatus: %d\n---\n", status);
    }
    freeCommands(allCommands);
  }

  free(userName);
  freeEnv(env);
}
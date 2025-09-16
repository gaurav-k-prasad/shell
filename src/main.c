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
  char *initialDirectory = getcwd(NULL, 0);
  char buff[1024];
  char **env = NULL;
  if (cloneEnv(envp, &env) == -1)
  {
    fprintf(stderr, "enviornment duplication failed\n");
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
      fprintf(stderr, "Lexical phase failed\n");
      continue;
    }

    Commands *allCommands = splitCommands(tokenVec);
    if (allCommands == NULL)
    {
      freeVecToken(tokenVec);
      fprintf(stderr, "Parsing failed\n");
      continue;
    }

    freeVecToken(tokenVec);

    for (int i = 0; i < allCommands->commands->size; i++)
    {
      Command *command = allCommands->commands->data[i];
      int status = executeCommand(command, &env, initialDirectory);
    }
    freeCommands(allCommands);
  }

  free(userName);
  freeEnv(env);
}
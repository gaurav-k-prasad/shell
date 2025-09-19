#include "../headers/myshell.h"

struct termios orig_termios;
void shellLoop(char **env);

int main(int argc, char const *argv[], char *envp[])
{
  (void)argc;
  (void)argv;

  struct sigaction sa;
  sa.sa_handler = handleSignal;           // Set the handler function.
  sa.sa_flags = 0;                        // Clear the flags. Crucially, this ensures SA_RESTART is OFF.
  sigemptyset(&sa.sa_mask);               // Block other signals while the handler is running.
  if (sigaction(SIGINT, &sa, NULL) == -1) // Register the signal handler using sigaction().
  {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  shellLoop(envp);
  return 0;
}

void shellLoop(char **envp)
{
  ForgettingDoublyLinkedList *history = createFDLL(100);
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    exit(EXIT_FAILURE);

  atexit(disableRawMode);
  enableRawMode();

  char *input = NULL; // input string
  char *initialDirectory = getcwd(NULL, 0);
  char **env = NULL;
  if (cloneEnv(envp, &env) == -1)
  {
    fprintf(stderr, "enviornment duplication failed\n");
    exit(EXIT_FAILURE);
  }
  char *userName = myGetenv("LOGNAME", env);

  while (1)
  {
    printShellStart(env, userName);
    input = getInputString(history);
    if (input == NULL)
      continue;

    VectorToken *tokenVec = getTokens(input, env);
    if (tokenVec == NULL)
    {
      fprintf(stderr, "Lexical phase failed\n");
      continue;
    }

    if (tokenVec->size > 0)
    {
      if (history->size > 0)
      {
        if (myStrcmp(input, history->tail->command) != 0)
          insertInFDLL(history, input);
      }
      else
      {
        insertInFDLL(history, input);
      }
    }
    free(input); // free the input string

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
      if (status > 0)
        break; // represents that the command was closed by ^C interrupt
    }
    freeCommands(allCommands);
  }

  freeFDLL(history);
  free(userName);
  freeEnv(env);
}
#include "../headers/myshell.h"

volatile int termCols;                       // tells about the columns in the terminal (width of column)
volatile sig_atomic_t whichSignal = INT_MIN; // initiate it with invalid signal
struct termios orig_termios;
ForgettingDoublyLinkedList *history = NULL;

void shellLoop(char **env);

int main(int argc, char const *argv[], char *envp[])
{
  (void)argc;
  (void)argv;

  struct sigaction sa;
  sa.sa_handler = handleSignal;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGCHLD, &sa, NULL);
  sigaction(SIGWINCH, &sa, NULL);

  /**
   * Whenever a child terminal is created by forking this process it inherits the stdout of this process
   * In that case whenever the command like ls & tries to write in the child terminal it inherently is trying
   * to write in the main/parent terminal which will be blocked until we ignore teh SIGTTOU
   * SIGTTOU - Signal Terminal Output for Background Process.
   */
  signal(SIGTTOU, SIG_IGN); // so background writes don’t stop

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  termCols = w.ws_col;

  shellLoop(envp);
  return 0;
}

void shellLoop(char **envp)
{
  history = createFDLL(100);
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

  int status = 0;
  while (1)
  {
    if (status == 0)
      printShellStart(env, userName);
    status = getInputString(history, &input);
    if (input == NULL || status != 0)
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
    input = NULL;

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
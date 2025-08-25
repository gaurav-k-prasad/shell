#include "myshell.h"

int executor(char **args, char **env)
{
  pid_t pid = fork();
  int childStatus;

  if (pid == -1)
  {
    fprintf(stderr, "fork failed");
    return 1;
  }
  else if (pid > 0) // parent process
  {
    waitpid(pid, &childStatus, 0); // wait for child process

    if (WIFSIGNALED(childStatus)) // if process exited with a signal
    // WIF -> Wait IF signaled -> eg. SIGKILL, SIGSEGV
    {
      int sig = WTERMSIG(childStatus); // extract signal number
      fprintf(stderr, "Child terminated by signal %d\n", sig);
    }
  }
  else // child process
  {
    char *cwd = getcwd(NULL, 0);
    // 1 for / and 1 for '\0'
    int len = (myStrlen(cwd) + 1 + myStrlen(args[0]) + 1);
    char *buff = malloc(sizeof(char) * len);
    snprintf(buff, len, "%s%s%s", cwd, PATH_SEPARATOR, args[0]);
    char *path = NULL;

    if (access(buff, X_OK) == 0)
    {
      path = buff;
    }

    if (path == NULL)
    {
      path = getFullPathOfWhich(args[0], env);
    }
    if (path == NULL)
    {
      fprintf(stderr, "%s is not a command\n", args[0]);
      _exit(127); // command not found code
    }

    execve(path, args, env); // if it returns then that's an error
    perror("execve failed");

    free(path);
    _exit(126); // command found but couldn't execute
  }

  return 0;
}
#include "../headers/myshell.h"

int executor(char ***commandArgs, char **env)
{
  // find the number of commands connected via pipe(|)
  int commandCount = 0;
  while (commandArgs[commandCount])
    commandCount++;

  int pipeCount = commandCount - 1;
  int fds[pipeCount][2];

  for (int i = 0; i < pipeCount; i++)
  {
    pipe(fds[i]);
  }

  // store all pids that will be forked
  int pids[commandCount];

  for (int i = 0; i < commandCount; i++)
  {
    int pid = fork();
    pids[i] = pid;

    if (pid == -1) // kill all the processes if fork failed
    {
      for (int j = 0; j < i; j++)
      {
        kill(pids[j], SIGTERM);
      }

      // reap them
      for (int j = 0; j < i; j++)
      {
        waitpid(pids[j], NULL, 0);
      }

      perror("fork failed");
      exit(1);
    }
    else if (pid == 0)
    {
      char *cwd = getcwd(NULL, 0);
      int len = myStrlen(cwd) + 1 + myStrlen(commandArgs[i][0]);                 // +1 for / for path joining
      char *buff = malloc(sizeof(char) * (len + 1));                             // +1 for \0
      snprintf(buff, len + 1, "%s%s%s", cwd, PATH_SEPARATOR, commandArgs[i][0]); // join paths
      char *path = NULL;

      if (access(buff, X_OK) == 0)
      {
        path = buff;
      }

      if (path == NULL)
      {
        path = getFullPathOfWhich(commandArgs[i][0], env); // try to find the process full path
      }
      if (path == NULL) // command not found
      {
        fprintf(stderr, "%s is not a command\n", commandArgs[i][0]);
        _exit(127); // command not found code
      }

      // handle pipes
      if (i != 0)
      {
        dup2(fds[i - 1][0], STDIN_FILENO); // duplicate input pipe
      }
      if (i != commandCount - 1)
      {
        dup2(fds[i][1], STDOUT_FILENO); // duplicate output pipe
      }
      for (int i = 0; i < pipeCount; i++) // close all pipes
      {
        close(fds[i][0]);
        close(fds[i][1]);
      }

      execve(path, commandArgs[i], env); // if it returns then that's an error
      perror("execve failed");

      free(path);
      _exit(126); // command found but couldn't execute
    }
  }

  for (int i = 0; i < pipeCount; i++) // close parent pipes
  {
    close(fds[i][0]);
    close(fds[i][1]);
  }

  for (int i = 0; i < commandCount; i++) // wait for child processes
  {
    waitpid(pids[i], NULL, 0);
  }

  return 0;
}
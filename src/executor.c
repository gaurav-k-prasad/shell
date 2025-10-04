#include "../headers/myshell.h"

int executeAllPipelines(Command *command, char ***env, char *initialDirectory, int *laststatus)
{
  for (int i = 0; i < command->pipelines->size; i++)
  {
    Pipeline *pipeline = command->pipelines->data[i];
    int status = executePipeline(pipeline, env, initialDirectory, command->isBackground);
    *laststatus = status;
    if (status < 0) // means that process was interrupted by ^C
    {
      return -1;
    }

    if (pipeline->separator == AND && status != 0 || pipeline->separator == OR && status == 0)
      break;
  }
  return 0;
}

int executeCommand(Command *command, char ***env, char *initialDirectory)
{
  int laststatus = 0;
  if (command->isBackground)
  {
    pid_t backgroundShell = fork();
    if (backgroundShell == -1)
    {
      return -1;
    }

    if (backgroundShell == 0) // child process
    {
      setpgid(0, 0);

      // reset the signals for the child terminal
      signal(SIGCHLD, SIG_DFL);
      signal(SIGINT, SIG_DFL);

      // executes all the pipelines
      executeAllPipelines(command, env, initialDirectory, &laststatus);

      // ideally should be in parent shell but...
      printf("Background process exit: %d\n", laststatus);
      fflush(stdout);
      _exit(laststatus);
    }
    else
    {
      // make the background shell it's own process group
      setpgid(backgroundShell, backgroundShell);

      printf("Process Group: %d\n", backgroundShell);
      fflush(stdout);
      return 0;
    }
  }
  else
  {
    // if foreground process
    executeAllPipelines(command, env, initialDirectory, &laststatus);
    return laststatus;
  }
}

int executePipeline(Pipeline *pipeline, char ***env, char *initialDirectory, bool isBackground)
{
  int pipelineComponentCount = pipeline->components->size;
  int pipeCount = pipelineComponentCount - 1;
  int fds[pipeCount][2];
  int pids[pipelineComponentCount];
  memset(pids, -1, sizeof(int) * pipelineComponentCount);

  // finding if the command is a builtin
  if (pipelineComponentCount > 0)
  {
    PipelineComponent *first = pipeline->components->data[0];
    char *command = first->tokens->data[0]->token;

    if (isBuiltin(command) || strcmp(command, "ai") == 0)
    {
      if (pipelineComponentCount > 1)
      {
        fprintf(stderr, "invalid command\n");
        return -1;
      }
    }
  }

  // set up pipes for IPC
  for (int i = 0; i < pipeCount; i++)
  {
    pipe(fds[i]);
  }

  int processGroup = INT_MIN;
  for (int i = 0; i < pipelineComponentCount; i++)
  {
    int status = executePipelineComponent(pipeline->components->data[i], env, fds, pipeCount, i, pids, initialDirectory, &processGroup, isBackground);
    if (i == 0 && !isBackground) // if not background then we can take terminal control
      if (processGroup != INT_MIN)
        tcsetpgrp(STDIN_FILENO, processGroup); // the process group gets the access of the terminal control

    if (status == -1)
    {
      killPids(0, i, pids); // kill all the processes till now if pipeline creation failed
      enableRawMode();
      if (!isBackground) // if background process, don't reclaim the terminal control
        tcsetpgrp(STDIN_FILENO, getpid());
      return -1;
    }
  }

  // close parent pipes
  closePipes(fds, pipeCount);

  int processGroupStatus = 0;
  int lastStatus = 0;
  // wait for child processes
  for (int j = 0; j < pipelineComponentCount; ++j)
  {
    if (pids[j] == -1)
      continue;

    pid_t w;
    int s;
    // if interrupted by random signal restart again
    while ((w = waitpid(pids[j], &s, 0)) == -1 && errno == EINTR)
      ;
    if (w == pids[j])
    {
      if (WIFEXITED(s))
        lastStatus = WEXITSTATUS(s);
      else if (WIFSIGNALED(s))
      {
        lastStatus = -1; // signaled
        write(STDOUT_FILENO, "\n", 1);
      }
    }
  }

  if (!isBackground) // if background process, don't reclaim the terminal control
    tcsetpgrp(STDIN_FILENO, getpid());
  enableRawMode();
  return lastStatus;
}

int executePipelineComponent(PipelineComponent *pc, char ***env, int fds[][2], int pipeCount, int i, int pids[], char *initialDirectory, int *processGroup, bool isBackground)
{
  Token **tokens = pc->tokens->data;
  int tokensLen = pc->tokens->size;
  int commandCount = pipeCount + 1;

  char *infile = NULL;
  char *outfile = NULL;
  int commandEnd = tokensLen;
  // from the given pipeline component find it's infile or outfile if any
  bool isAppendOutputFile = findInOutFileAndCommandEnd(pc, &infile, &outfile, &commandEnd);
  int parentRetValue = 0;
  fflush(stdout);

  int infilefd = -1;
  int outfilefd = -1;
  if (infile)
  {
    infilefd = open(infile, O_RDONLY);
    if (infilefd == -1)
    {
      perror("open");
      parentRetValue = -1;
      goto closeFiles;
    }
  }
  if (outfile)
  {
    int fileOpenMode = isAppendOutputFile ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
    outfilefd = open(outfile, fileOpenMode, 0666);
    if (outfilefd == -1)
    {
      perror("open");
      parentRetValue = -1;
      goto closeFiles;
    }
  }

  // printf("commandEnd: %d - infile: %s, outfile: %s\n========\n", commandEnd, infile, outfile);

  // get the args in the pipeline component to give to execve
  char **args = malloc(sizeof(char *) * (commandEnd + 1));
  if (!args)
  {
    parentRetValue = -1;
    goto closeFiles;
  }

  for (int i = 0; i < commandEnd; i++)
  {
    args[i] = tokens[i]->token;
  }
  args[commandEnd] = NULL;

  // if it's the only builtin then execute it. no need to fork
  if (i == 0 && isBuiltin(args[0]))
  {
    parentRetValue = handleBuiltin(args, env, initialDirectory);
    free(args);
    goto closeFiles;
  }

  pid_t pid = fork();
  if (pid == -1)
  {
    free(args);
    perror("fork");
    parentRetValue = -1;
    goto closeFiles;
  }
  else if (pid == 0)
  {
    if (*processGroup == INT_MIN) // first process in the pipeline
    {
      setpgid(0, 0);
    }
    else
      setpgid(0, *processGroup); // following processes added to previous group

    if (!isBackground) // background processes won't have access to terminal
      disableRawMode();
    signal(SIGINT, SIG_DFL);

    int exitError = 0;

    // if infile or outfile connect them
    if (infile)
    {
      int status = dup2(infilefd, STDIN_FILENO);
      if (status == -1)
      {
        exitError = 1;
        goto exitChild;
      }
    }
    if (outfile)
    {
      int status = dup2(outfilefd, STDOUT_FILENO);
      if (status == -1)
      {
        exitError = 1;
        goto exitChild;
      }
    }

    // connect rest of the pipes
    if (i != 0)
    {
      int status = dup2(fds[i - 1][0], STDIN_FILENO); // duplicate input pipe
      if (status == -1)
      {
        exitError = 1;
        goto exitChild;
      }
    }
    if (i != commandCount - 1)
    {
      int status = dup2(fds[i][1], STDOUT_FILENO); // duplicate output pipe
      if (status == -1)
      {
        exitError = 1;
        goto exitChild;
      }
    }

  exitChild:
    // close all pipes as necessary ones are already duped above
    closePipes(fds, pipeCount);

    // close the files if opened
    if (infile)
      close(infilefd);
    if (outfile)
      close(outfilefd);

    if (exitError)
    {
      free(args);
      _exit(exitError);
    }

    // handle builtins
    if (isBuiltin(args[0]))
    {
      int status = handleBuiltin(args, env, initialDirectory);
      free(args);
      _exit(status);
    }

    // handle my implemented builtins
    if (isMyImplementedBuiltin(args[0]))
    {
      int status = handleMyImplementedBuiltin(args, env, initialDirectory);
      free(args);
      _exit(status);
    }

    // if not builtin
    char *path = getFullPathOfWhich(tokens[0]->token, *env); // try to find the process full path

    if (!path)
    {
      if (i > 0 || strncmp(tokens[0]->token, "./", myStrlen("./")) == 0) // if first command in the pipeline and it's a file then it must start with ./
      {
        char *cwd = getcwd(NULL, 0);
        int len = myStrlen(cwd) + myStrlen(PATH_SEPARATOR) + myStrlen(tokens[0]->token); // +1 for / for path joining
        char *buff = malloc(sizeof(char) * (len + 1));                                   // +1 for \0
        if (!buff)
        {
          free(cwd);
          free(args);
          _exit(1);
        }
        snprintf(buff, len + 1, "%s%s%s", cwd, PATH_SEPARATOR, tokens[0]->token); // join paths
        free(cwd);

        if (access(buff, X_OK) == 0)
        {
          path = buff;
        }
        else
        {
          free(buff);
        }
      }
    }

    if (!path) // command not found
    {
      fprintf(stderr, "%s: command not found\n", tokens[0]->token);
      _exit(127); // command not found code
    }

    execve(path, args, *env); // if it returns then that's an error
    perror("execve");

    free(path);
    free(args);
    _exit(126); // command found but couldn't execute
  }
  else // parent
  {
    // from the parent side also set up the process group to avoid race condition
    if (*processGroup == INT_MIN)
    {
      setpgid(pid, pid);
      *processGroup = pid;
    }
    else
    {
      setpgid(pid, *processGroup);
    }
    // give the parent the process id of the child
    pids[i] = pid;
  closeFiles:
    // parent closing all infiles/outfiles it opened
    if (infile && infilefd != -1)
      close(infilefd);
    if (outfile && outfilefd != -1)
      close(outfilefd);

    return parentRetValue;
  }
}

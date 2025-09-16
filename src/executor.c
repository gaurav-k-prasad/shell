#include "../headers/myshell.h"

int executeCommand(Command *command, char ***env, char *initialDirectory)
{
  int laststatus = 0;
  for (int i = 0; i < command->pipelines->size; i++)
  {
    Pipeline *pipeline = command->pipelines->data[i];
    int status = executePipeline(pipeline, env, initialDirectory);
    laststatus = status;
    if (status > 0) // means that process was interrupted by ^C
    {
      return status;
    }

    if (pipeline->separator == AND && status != 0)
    {
      break;
    }
    else if (pipeline->separator == OR)
    {
      if (status == 0)
      {
        break;
      }
    }
  }
  return laststatus;
}

int executePipeline(Pipeline *pipeline, char ***env, char *initialDirectory)
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

    if (isBuiltin(command))
    {
      if (pipelineComponentCount > 1)
      {
        fprintf(stderr, "invalid command\n");
        return -1;
      }
    }
  }

  for (int i = 0; i < pipeCount; i++)
  {
    pipe(fds[i]);
  }

  for (int i = 0; i < pipelineComponentCount; i++)
  {
    int status = executePipelineComponent(pipeline->components->data[i], env, fds, pipeCount, i, pids, initialDirectory);
  }

  // close parent pipes
  closePipes(fds, pipeCount);

  int pipelineStatus = 0;
  for (int i = 0; i < pipelineComponentCount; i++)
  {
    if (pids[i] == -1) // if not a valid child process or did not fork
      continue;

    int processStatus = 0;
    // in case it's interrupted by a interrupt the waitpid sys call won't be restarted
    // as SA_RESTART in SIGINT handling is 0 so we manually restart it
    while (waitpid(pids[i], &processStatus, 0) == -1)
    {
      if (errno == EINTR)
        continue; // interrupted by signal, retry
      else
        break; // real error, stop
    }

    if (i == pipelineComponentCount - 1)
    {
      if (WIFEXITED(processStatus))
      {
        pipelineStatus = WEXITSTATUS(processStatus); // 0 = success
      }
      else if (WIFSIGNALED(processStatus))
      {
        pipelineStatus = WTERMSIG(processStatus);
        for (int j = i; j < pipelineComponentCount; j++)
        {
          if (pids[j] > 0)
          {
            kill(pids[j], SIGKILL);
          }
        }
        return pipelineStatus; // ^C interrupted the process
      }
      else
      {
        pipelineStatus = -1;
      }
    }
  }

  return pipelineStatus;
}

int executePipelineComponent(PipelineComponent *pc, char ***env, int fds[][2], int pipeCount, int i, int pids[], char *initialDirectory)
{
  Token **tokens = pc->tokens->data;
  int tokensLen = pc->tokens->size;
  int commandCount = pipeCount + 1;

  char *infile = NULL;
  char *outfile = NULL;
  int commandEnd = tokensLen;
  findInOutFileAndCommandEnd(pc, &infile, &outfile, &commandEnd);
  int parentRetValue = 0;

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
    outfilefd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (outfilefd == -1)
    {
      perror("open");
      parentRetValue = -1;
      goto closeFiles;
    }
  }

  // printf("commandEnd: %d - infile: %s, outfile: %s\n========\n", commandEnd, infile, outfile);

  char **args = malloc(sizeof(char *) * (commandEnd + 1));
  if (!args)
  {
    parentRetValue = -1;
    goto closeFiles;
  }

  for (int i = 0; i < commandEnd; i++)
    args[i] = tokens[i]->token;
  args[commandEnd] = NULL;

  // if it's the only builtin then execute it. no need to fork
  if (i == 0 && isBuiltin(args[0]))
  {
    parentRetValue = handleBuiltin(args, env, initialDirectory);
    free(args);
    goto closeFiles;
  }

  int pid = fork();
  if (pid == -1)
  {
    free(args);
    perror("fork");
    parentRetValue = -1;
    goto closeFiles;
  }

  pids[i] = pid;

  if (pid == 0)
  {
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
    // close all pipes
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

closeFiles:
  // parent closing all infiles/outfiles it opened
  if (infile && infilefd != -1)
    close(infilefd);
  if (outfile && outfilefd != -1)
    close(outfilefd);

  return parentRetValue;
}

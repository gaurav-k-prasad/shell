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

int executeCommand(Command *command, char ***env, char *initialDirectory)
{
  for (int i = 0; i < command->pipelines->size; i++)
  {
    int status = executePipeline(command->pipelines->data[i], env, initialDirectory);

    // todo: handle && ||
  }
  return 0;
}

int executePipeline(Pipeline *pipeline, char ***env, char *initialDirectory)
{
  int pipelineComponentCount = pipeline->components->size;
  int pipeCount = pipelineComponentCount - 1;
  int fds[pipeCount][2];
  int pids[pipelineComponentCount];

  // finding if the command is a builtin
  if (pipelineComponentCount > 0)
  {
    PipelineComponent *first = pipeline->components->data[0];
    char *command = first->tokens->data[0]->token;

    if (isBuiltin(command))
    {
      if (pipelineComponentCount > 1)
      {
        fprintf(stderr, "invalid pipeline\n");
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

    // todo: handle status
  }

  // close parent pipes
  closePipes(fds, pipeCount);

  for (int i = 0; i < pipelineComponentCount; i++)
  {
    waitpid(pids[i], NULL, 0);
  }

  return 0;
}

int executePipelineComponent(PipelineComponent *pc, char ***env, int fds[][2], int pipeCount, int i, int pids[], char *initialDirectory)
{
  Token **tokens = pc->tokens->data;
  int tokensLen = pc->tokens->size;
  int commandCount = pipeCount + 1;

  char *infile = NULL;
  char *outfile = NULL;
  int commandEnd = tokensLen;
  findInOutFileAndCommandEnd(pc, tokensLen, tokens, &infile, &outfile, &commandEnd);

  int infilefd = -1;
  int outfilefd = -1;
  if (infile)
    infilefd = open(infile, O_RDONLY);
  if (outfile)
    outfilefd = open(outfile, O_RDWR | O_CREAT);

  printf("commandEnd: %d - infile: %s, outfile: %s\n========\n", commandEnd, infile, outfile);

  char **args = malloc(sizeof(char *) * (commandEnd + 1));
  for (int i = 0; i < commandEnd; i++)
    args[i] = tokens[i]->token;
  args[commandEnd] = NULL;

  // if it's the only builtin then execute it. no need to fork
  if (i == 0 && isBuiltin(args[0]))
  {
    handleBuiltin(args, env, initialDirectory);
    return 0;
  }

  int pid = fork();
  // todo: error: forking failed
  pids[i] = pid;

  if (pid == 0)
  {
    // if infile or outfile connect them
    if (infile)
      dup2(infilefd, STDIN_FILENO);

    if (outfile)
      dup2(outfilefd, STDOUT_FILENO);

    // connect rest of the pipes
    if (i != 0)
    {
      dup2(fds[i - 1][0], STDIN_FILENO); // duplicate input pipe
    }
    if (i != commandCount - 1)
    {
      dup2(fds[i][1], STDOUT_FILENO); // duplicate output pipe
    }

    // close all pipes
    closePipes(fds, pipeCount);

    // close the files if opened
    if (infile)
      close(infilefd);
    if (outfile)
      close(outfilefd);

    // handle builtins
    if (isBuiltin(args[0]))
    {
      handleBuiltin(args, env, initialDirectory);
      exit(0);
    }

    // handle my implemented builtins
    if (isMyImplementedBulitin(args[0]))
    {
      handleMyImplementedBulitin(args, env, initialDirectory);
      exit(0);
    }

    // if not builtin
    char *cwd = getcwd(NULL, 0);
    int len = myStrlen(cwd) + 1 + myStrlen(tokens[0]->token);                 // +1 for / for path joining
    char *buff = malloc(sizeof(char) * (len + 1));                            // +1 for \0
    snprintf(buff, len + 1, "%s%s%s", cwd, PATH_SEPARATOR, tokens[0]->token); // join paths
    char *path = NULL;

    if (access(buff, X_OK) == 0)
    {
      path = buff;
    }

    if (path == NULL)
    {
      path = getFullPathOfWhich(tokens[0]->token, *env); // try to find the process full path
    }
    if (path == NULL) // command not found
    {
      fprintf(stderr, "%s is not a command\n", tokens[0]->token);
      _exit(127); // command not found code
    }

    execve(path, args, *env); // if it returns then that's an error
    perror("execve failed");

    free(path);
    _exit(126); // command found but couldn't execute
  }

  // parent closing all infiles/outfiles it opened
  if (infile)
    close(infilefd);
  if (outfile)
    close(outfilefd);

  return 0;
}

#include "../headers/myshell.h"

int commandCd(char **args, char *initialDirectory)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "expected an argument: cd [dir]\n");
    return -1;
  }

  if (chdir(args[1]) != 0)
  {
    perror("cd");
    return -1;
  }

  return 0;
}

int commandPwd()
{
  char *cwd = getcwd(NULL, 0);
  if (cwd == NULL)
  {
    return -1;
  }
  fprintf(stdout, "%s\n", cwd);
  free(cwd);
  return 0;
}

int commandEcho(char **args, char **env)
{
  int printNewLine = true;
  int start = 1;
  if (args[1] && myStrcmp(args[1], "-n") == 0) // handling -n flag for no new line character
  {
    start = 2;
    printNewLine = false;
  }

  for (int i = start; args[i]; i++) // parse all the arguments for echo
  {
    fprintf(stdout, "%s", args[i]);
    if (args[i + 1])
    {
      fprintf(stdout, " ");
    }
  }

  if (printNewLine)
    fprintf(stdout, "\n");

  return 0;
}

int commandSource(char **args, char ***env, char *initialDirectory)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "source: filename required\n");
    return -1;
  }

  FILE *f = fopen(args[1], "r");
  if (!f)
  {
    perror("source");
    return -1;
  }

  char input[1024];
  while (fgets(input, sizeof(input), f))
  {
    VectorToken *tokenVec = getTokens(input, *env);
    if (tokenVec == NULL)
    {
      fprintf(stderr, "Lexical phase failed\n");
      continue;
    }
    Commands *allCommands = splitCommands(tokenVec);
    if (allCommands == NULL)
    {
      freeVecToken(tokenVec);
      continue;
    }
    freeVecToken(tokenVec);

    for (int i = 0; i < allCommands->commands->size; i++)
    {
      Command *command = allCommands->commands->data[i];
      int status = executeCommand(command, env, initialDirectory);
      if (status > 0)
        break; // represents that the command was closed by ^C interrupt
    }
    freeCommands(allCommands);
  }

  fclose(f);
  return 0;
}

int commandEnv(char **env)
{
  fprintf(stdout, "%-40s | %s\n", "Enviornment Variable", "Value");
  fprintf(stdout, "-------------------------------------------------------------------------------------------\n");
  if (env == NULL)
    return -1;

  for (int i = 0; env[i]; i++)
  {
    int count = 0;
    for (int j = 0; env[i][j]; j++)
    {
      if (env[i][j] == '=')
        break;
      count++;
    }

    char *buff = (char *)malloc(sizeof(char) * (count + 1));
    if (buff == NULL)
    {
      return -1;
    }
    strncpy(buff, env[i], count); // get value of the enviornment variable to print
    buff[count] = '\0';

    fprintf(stdout, "%-40s | %s\n", buff, env[i] + count + 1); // formatted print key and value
    free(buff);
  }

  return 0;
}

int commandAI(char **args)
{
  printf("\033[31mAI never changes the state of current shell\033[0m\n");
  for (int i = 0; i < MAX_AI_ATTEMPTS; i++)
  {
    int pid = fork();
    if (pid == -1)
      return -1; // fork failed

    if (pid == 0) // child process
    {
      char *a[] = {"python3", "./AI/interface.py", NULL};
      execvp(a[0], a);
      perror("execvp failed"); // only runs if exec fails
      exit(127);               // command not found
    }

    // parent
    int status;
    while (waitpid(pid, &status, 0) == -1 && errno == EINTR)
      ;
    int exitCode = 0;

    if (WIFEXITED(status))
    {
      if (WEXITSTATUS(status) != 0)
        return -1;

      AICommands *commands = NULL;
      AIQuestions *questions = NULL;
      int status = parseAI(&commands, &questions, "ai_output.txt");

      if (status == -1)
        return -1;

      if (commands)
      {
        int totalCommandLength = 0;
        if (commands->commandsCount > 0)
        {
          printf("\nCommands: \n");
          for (int i = 0; i < commands->commandsCount; i++)
          {
            printf("- %s\n", commands->commands[i]);
            totalCommandLength += strlen(commands->commands[i]);
          }
        }
        printf("\n\033[32mExplanation: %s\033[0m\n", commands->explanation);
        printf("\n\033[31mWarning: %s\033[0m\n\n", commands->warning);
        printf("Do you want to execute the commands (y/[n]): ");

        char input = 'n';
        input = getchar();
        if (input != '\n')
          getchar();
        if (input != 'y' && input != '\n')
          return -1;

        // execute the commands
        int allocSize = sizeof(char) * (totalCommandLength + commands->commandsCount + 1); // count for ;
        char *commandStr = (char *)malloc(allocSize);
        if (commandStr == NULL)
        {
          exitCode = -1;
          goto freeAIStructs;
        }
        commandStr[0] = '\0';

        FILE *fp = fopen("ai_temp.sh", "w");
        if (!fp)
        {
          exitCode = -1;
          free(commandStr);
          goto freeAIStructs;
        }

        for (int i = 0; i < commands->commandsCount; i++)
        {
          strcat(commandStr, commands->commands[i]);
          if (i < commands->commandsCount - 1)
            strcat(commandStr, ";");
        }

        fprintf(fp, "(%s)2> >(tee err.log)", commandStr);
        free(commandStr);
        fclose(fp);

        int bashExecutePid = fork();

        if (bashExecutePid == -1)
        {
          exitCode = -1;
          goto freeAIStructs;
        }
        else if (bashExecutePid == 0)
        {
          char *args[] = {"bash", "ai_temp.sh", NULL};
          execvp("bash", args);
          perror("execvp");
          _exit(errno); // return errno to parent
        }
        else // parent
        {
          int status;
          waitpid(bashExecutePid, &status, 0);

          if (WIFEXITED(status))
          {
            int code = WEXITSTATUS(status);
            if (code == 0) // if all commands succeeded
              return 0;
            else
              continue;
          }
          else if (WIFSIGNALED(status))
          {
            return -1;
          }
        }
      }
      else if (questions)
      {
        printf("\nAnswer these questions to get accurate results: \n");
        for (int i = 0; i < questions->questionsCount; i++)
          printf("- %s\n", questions->questions[i]);
        printf("\n\033[32mExplanation: %s\033[0m\n", questions->explanation);
      }
      else
      {
        return -1;
      }

    freeAIStructs:
      freeAICommands(commands);
      freeAIQuestions(questions);

      return exitCode;
    }
    else if (WIFSIGNALED(status))
    {
      fprintf(stderr, "AI command killed by signal %d\n", WTERMSIG(status));
      return -1;
    }
  }

  return -1; // all attempts failed
}

int commandWhich(char **args, char **env)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "which: expected a filename: which [file]\n");
    return -1;
  }

  // if one of these then it's a builtin command
  const char *builtIns[] = {"cd", "pwd", "echo", "env", "unsetenv", "setenv", "exit", "which"};

  for (int i = 0; i < sizeof(builtIns) / sizeof(builtIns[0]); i++)
  {
    if (myStrcmp(builtIns[i], args[1]) == 0)
    {
      fprintf(stdout, "%s: shell built-in command\n", builtIns[i]);
      return 0;
    }
  }

  // look through everywhere in env PATH where the command is
  char *fullpath = getFullPathOfWhich(args[1], env);
  if (fullpath == NULL)
  {
    fprintf(stdout, "%s: command not found\n", args[1]);
    return -1;
  }

  fprintf(stdout, "%s\n", fullpath);
  free(fullpath);
  return 0;
}

char *getFullPathOfWhich(char *command, char **env)
{
  // Locate the path enviornment variable
  char *pathEnv = myGetenv("PATH", env);
  if (!pathEnv)
  {
    return NULL;
  }

  // split the PATH by separator
  char *tok = strtok(pathEnv, SEP);

  while (tok)
  {
    size_t len = myStrlen(tok) + 1 + myStrlen(command) + 1; // +1 for / and +1 for '\0'
    char *buff = (char *)malloc(sizeof(char) * len);
    if (!buff)
    {
      free(pathEnv);
      return NULL;
    }

    if (tok[len - 1] != *PATH_SEPARATOR)
      snprintf(buff, len, "%s%s%s", tok, PATH_SEPARATOR, command);
    else
      snprintf(buff, len, "%s%s", tok, command);

    // find the binary file if it exists or not in the given path
    if (access(buff, X_OK) == 0)
    {
      free(pathEnv);
      return buff; // caller must free
    }

    free(buff);
    tok = strtok(NULL, SEP);
  }

  free(pathEnv);
  return NULL;
}

int commandExport(char **args, char ***envReference)
{
  char **env = *envReference;
  if (!args[1])
  {
    fprintf(stderr, "export: expected VAR=value\n");
    return -1;
  }

  // count and allocate new enviornment variable string
  int envCount = 0;
  while (env[envCount])
    envCount++;
  char **newEnv = (char **)malloc(sizeof(char *) * (envCount + 2)); // 1 for null 1 for new env
  if (newEnv == NULL)
  {
    return -1;
  }

  // get the new enviornment variable - variable name
  char *str = args[1];
  int varnamelen = 0;

  while (*str && *str != '=')
  {
    str++;
    varnamelen++;
  }

  // making sure new enviornment variable has a value associated with it
  if (*str != '=')
  {
    free(newEnv);
    return -1;
  }

  // actually copy the varname
  char *varname = (char *)malloc(sizeof(char) * (varnamelen + 1));
  if (varname == NULL)
  {
    free(newEnv);
    return -1;
  }
  strncpy(varname, args[1], varnamelen);
  varname[varnamelen] = '\0';
  str++; // skip '='

  int k = 0; // pointer for newEnv variable
  size_t len;
  char *curr;
  newEnv[k] = NULL;

  for (int i = 0; env[i]; i++)
  {
    // copy the others into newEnv
    if (!(myStrnicmp(env[i], varname, varnamelen) == 0 && env[i][varnamelen] == '='))
    {
      len = myStrlen(env[i]);
      curr = (char *)malloc(sizeof(char) * (len + 1));
      if (!curr) // malloc failed and delete everything
      {
        free(varname);
        freeEnv(newEnv);
        return -1;
      }
      myStrcpy(curr, env[i]);
      newEnv[k++] = curr;
      newEnv[k] = NULL;
    }
  }

  // copy the new enviornment variable
  len = myStrlen(args[1]);                         // 1 for '='
  curr = (char *)malloc(sizeof(char) * (len + 1)); // 1 for \0
  if (!curr)
  {
    free(varname);
    freeEnv(newEnv);
    return -1;
  }

  snprintf(curr, len + 1, "%s", args[1]);
  newEnv[k++] = curr;
  newEnv[k] = NULL;

  // free the memory
  free(varname);
  freeEnv(env);
  *envReference = newEnv;

  return 0;
}

int commandUnset(char **args, char ***envReference)
{
  char **env = *envReference;
  if (args == NULL || args[1] == NULL || env == NULL)
  {
    fprintf(stderr, "expected argument unset [env]");
    return -1;
  }

  // count env
  size_t envCount = 0;
  while (env[envCount])
    envCount++;

  if (envCount == 0)
  {
    return 0; // nothing to unset
  }

  // in case variable not removed - makes sure there's always space for NULL
  char **newEnv = malloc(sizeof(char *) * (envCount + 1)); // removing 1 and a adding null terminator
  if (!newEnv)
  {
    return -1;
  }
  newEnv[0] = NULL;

  bool found = false;

  char *varName = args[1];
  size_t varLen = myStrlen(varName);
  int k = 0;

  for (int i = 0; env[i]; i++)
  {
    if (!found && (myStrnicmp(env[i], varName, varLen) == 0) && (env[i][varLen] == '='))
    {
      // found the enviornment variable
      found = true;
      continue;
    }

    size_t len = myStrlen(env[i]);
    newEnv[k] = (char *)malloc(sizeof(char) * (len + 1)); // +1 for '\0'
    if (!newEnv[k])
    {
      freeEnv(newEnv);
      return -1;
    }

    strcpy(newEnv[k], env[i]);
    k++;
    newEnv[k] = NULL;
  }

  if (!found)
  {
    freeEnv(newEnv);
    return 0;
  }

  freeEnv(env);
  *envReference = newEnv;
  return 0;
}

int cloneEnv(char **env, char ***newEnvReference)
{
  if (env == NULL)
  {
    *newEnvReference = NULL;
    return -1;
  }

  // count and allocate new enviornment variable string
  int envCount = 0;
  while (env[envCount])
  {
    envCount++;
  }
  char **newEnv = (char **)malloc(sizeof(char *) * (envCount + 1)); // 1 for null
  if (newEnv == NULL)
  {
    *newEnvReference = NULL;
    return -1;
  }
  int k = 0; // pointer for newEnv variable
  size_t len;
  char *curr;
  newEnv[0] = NULL;

  for (int i = 0; env[i]; i++)
  {
    // clone everything
    len = myStrlen(env[i]);
    curr = (char *)malloc(sizeof(char) * (len + 1));
    if (!curr)
    {
      freeEnv(newEnv);
      *newEnvReference = NULL;
      return -1;
    }
    myStrcpy(curr, env[i]);
    newEnv[k++] = curr;
    newEnv[k] = NULL;
  }
  *newEnvReference = newEnv;
  return 0;
}

void freeEnv(char **env)
{
  if (env == NULL)
    return;

  for (int i = 0; env[i]; i++)
  {
    free(env[i]);
  }
  free(env);
}

#include "../headers/gshell.h"
extern ForgettingDoublyLinkedList *history;

#define AI_OUTPUT_FILE "gshellAIOutputInfo.txt"
#define AI_ERROR_FILE "gshellAIErrorInfo.txt"
#define AI_HISTORY_FILE "gshellAIHistoryInfo.txt"
#define AI_PLATFORM_FILE "gshellAIPlatformInfo.txt"

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
  printf("\033[31mAI never changes the state of current shell\033[0m\n\n");

  if (writeNHistoryInfoToFile(AI_HISTORY_FILE, AI_HISTORY_COUNT, history) == -1)
    return -1;
  if (writePlatformInfoToFile(AI_PLATFORM_FILE) == -1)
    return -1;
  if (writeErrorInfoToFile(AI_ERROR_FILE, "") == -1) // creating the file
    return -1;

  char *questionAnswer = NULL; // if any question asked by ai would be answered as this
  char *aiQuery = (char *)malloc(sizeof(char) * MAX_AI_QUERY_LEN);
  if (!aiQuery)
    return -1;
  aiQuery[0] = '\0';

  for (int i = 1; args[i]; i++) // don't want to include "ai" so starting from 1
  {
    strcat(aiQuery, args[i]);
    if (args[i + 1])
      strcat(aiQuery, " ");
  }

  AICommands *commands = NULL; // containers for storing ai output
  AIQuestions *questions = NULL;
  int exitCode = 0;

  for (int i = 0; i < MAX_AI_ATTEMPTS; i++)
  {
    int pid = fork();
    if (pid == -1)
    {
      exitCode = -1;
      goto exitAI; // fork failed
    }

    if (questionAnswer)
    {
      strcat(aiQuery, "; Clarification: ");
      strcat(aiQuery, questionAnswer);
      free(questionAnswer);
      questionAnswer = NULL;
    }

    if (pid == 0) // child process
    {
      char *pythonArgs[] = {"python3", "./AI/interface.py", aiQuery, NULL};

      execvp(pythonArgs[0], pythonArgs);
      perror("execvp failed"); // only runs if exec fails
      _exit(127);              // command not found
    }

    // parent
    int status;
    while (waitpid(pid, &status, 0) == -1 && errno == EINTR)
      ;

    if (WIFEXITED(status))
    {
      if (WEXITSTATUS(status) != 0)
      {
        exitCode = -1;
        goto exitAI;
      }
      freeAICommands(commands);
      freeAIQuestions(questions);
      commands = NULL;
      questions = NULL;
      if (parseAI(&commands, &questions, AI_OUTPUT_FILE) == -1)
      {
        exitCode = -1;
        goto exitAI;
      }

      printf("-----Attempt %d/%d------\n", i + 1, MAX_AI_ATTEMPTS);
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
        if (commands->commandsCount == 0)
          goto exitAI;

        printf("Do you want to execute the commands (y/[n]): ");

        char input = 'n';
        input = getchar();
        if (input != '\n')
          getchar();
        if (input != 'y' && input != '\n')
        {
          exitCode = -1;
          goto exitAI;
        }

        printf("\n");
        // execute the commands
        int allocSize = sizeof(char) * (totalCommandLength + (2 * commands->commandsCount) + 1); // count for && as delimiterp
        char *commandStr = (char *)malloc(allocSize);
        if (commandStr == NULL)
        {
          exitCode = -1;
          goto exitAI;
        }
        commandStr[0] = '\0';

        FILE *fp = fopen("gshellAITemp.sh", "w");
        if (!fp)
        {
          exitCode = -1;
          free(commandStr);
          goto exitAI;
        }

        for (int i = 0; i < commands->commandsCount; i++)
        {
          strcat(commandStr, commands->commands[i]);
          strcat(commandStr, "\n");
        }
        printf("%s\n", commandStr);
        fprintf(fp, "(%s)2> >(tee %s)", commandStr, AI_ERROR_FILE);
        free(commandStr);
        fclose(fp);

        int bashExecutePid = fork();

        if (bashExecutePid == -1)
        {
          exitCode = -1;
          goto exitAI;
        }
        else if (bashExecutePid == 0)
        {
          char *args[] = {"bash", "gshellAITemp.sh", NULL};
          execvp("bash", args);
          perror("execvp");
          _exit(errno); // return errno to parent
        }
        else // parent
        {
          int status;
          while (waitpid(bashExecutePid, &status, 0) == -1 && errno == EINTR)
            ;

          if (WIFEXITED(status))
          {
            int code = WEXITSTATUS(status);
            if (code == 0) // if all commands succeeded
            {
              exitCode = 0;
              goto exitAI;
            }
            else
              continue;
          }
          else if (WIFSIGNALED(status))
          {
            exitCode = -1;
            goto exitAI;
          }
        }
      }
      else if (questions)
      {
        printf("\nAnswer these questions to get accurate results: \n");
        for (int i = 0; i < questions->questionsCount; i++)
          printf("- %s\n", questions->questions[i]);
        printf("\n\033[32mExplanation: %s\033[0m\n", questions->explanation);

        printf("Answer: \n");
        size_t n = 0;
        int nRead = getline(&questionAnswer, &n, stdin);
        if (nRead == -1)
          continue;
        if (questionAnswer[nRead - 1] == '\n')
          questionAnswer[nRead - 1] = '\0';
        continue;
      }
      else
      {
        exitCode = -1;
        goto exitAI;
      }
    }
    else if (WIFSIGNALED(status))
    {
      fprintf(stderr, "AI command killed by signal %d\n", WTERMSIG(status));
      exitCode = -1;
      goto exitAI;
    }
  }

exitAI:
  free(aiQuery);
  freeAICommands(commands);
  freeAIQuestions(questions);
  return exitCode;
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

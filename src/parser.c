#include "../headers/myshell.h"

// Deprecated - not used
char **parseInput(char *input)
{
  if (!input)
  {
    return NULL;
  }

  char **tokens = (char **)malloc(MAX_INPUT_BUF * sizeof(char *));
  if (!tokens)
  {
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
  }

  char *token = NULL;
  size_t position = 0;
  size_t tokenLength = 0;
  tokens[0] = NULL;

  for (size_t i = 0; input[i] != '\0';)
  {
    if (input[i] == ' ' || input[i] == '\n')
    {
      i++;
      continue;
    }

    token = &input[i];

    while (input[i] && input[i] != ' ' && input[i] != '\n')
    {
      tokenLength++;
      i++;
    }

    tokens[position] = malloc((tokenLength + 1) * sizeof(char)); // +1 for NULL

    if (!tokens[position])
    {
      perror("malloc failed");
      // freeTokens(tokens);
      exit(EXIT_FAILURE);
    }

    for (size_t j = 0; j < tokenLength; j++)
    {
      tokens[position][j] = token[j];
    }
    tokens[position][tokenLength] = '\0'; // null terminate the token
    position++;
    tokens[position] = NULL; // NULL terminate array
    tokenLength = 0;         // reset token length
    token = NULL;
  }

  return tokens;
}

char ***extendedParser(char *input)
{
  int pipeCount = 0; // find number of pipes
  for (int i = 0; input[i]; i++)
  {
    if (input[i] == '|')
      pipeCount++;
  }

  char commandCount = pipeCount + 1;
  char **commands = (char **)malloc(sizeof(char *) * (commandCount + 1)); // 1 extra for null
  if (!commands)
  {
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
  }

  char *tok = strtok(input, "|"); // actually separate the commands
  for (int i = 0; tok; i++)
  {
    commands[i] = tok;
    tok = strtok(NULL, "|");
  }
  commands[commandCount] = NULL; // last command set null

  // in each command separate the arguments
  char ***commandArgs = (char ***)malloc(sizeof(char *) * (commandCount + 1)); // +1 for null
  if (!commandArgs)
  {
    free(commands);
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; commands[i]; i++)
  {
    char *tok = commands[i];
    char **tokens = (char **)malloc(sizeof(char *) * 1024);
    if (!commands)
    {
      for (int j = 0; j < i; j++)
      {
        free(commandArgs[j]);
      }
      free(commandArgs);
      free(commands);
      fprintf(stderr, "malloc failed\n");
      exit(EXIT_FAILURE);
    }
    char *arg = strtok(tok, " ");

    int j;
    for (j = 0; arg; j++) // split the command
    {
      tokens[j] = arg;
      arg = strtok(NULL, " ");
    }
    tokens[j] = NULL;
    commandArgs[i] = tokens;
  }

  commandArgs[commandCount] = NULL;
  return commandArgs;
}

char *parseString(char *str, char **env)
{
  if (str == NULL)
    return NULL;

  char *res = (char *)malloc(sizeof(char) * 4096);
  if (res == NULL)
  {
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
  }
  int i = 0;
  int resIndex = 0;

  while (str[i])
  {
    if (str[i] == '$')
    {
      i++;
      char buff[256];
      int j = 0;

      while (str[i] && (isalnum(str[i]) || str[i] == '_'))
        buff[j++] = str[i++];
      buff[j] = '\0';

      char *envVal = myGetenv(buff, env);
      if (envVal == NULL)
        continue;

      int temp = 0;
      while (envVal[temp])
      {
        res[resIndex++] = envVal[temp++];
      }
      free(envVal);
    }
    else
    {
      res[resIndex++] = str[i++];
    }
  }
  res[resIndex] = '\0';

  return res;
}

// Free allocated memory
void freeTokens(char ***tokens)
{
  if (!tokens)
  {
    return;
  }

  for (int i = 0; tokens[i]; i++)
  {
    free(tokens[i]);
  }

  free(tokens);
}

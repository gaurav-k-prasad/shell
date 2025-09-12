#include "../headers/myshell.h"

Token **getTokens(char *input, char **env)
{
  int i = 0;
  Token **allTokens = (Token **)malloc(sizeof(Token *) * MAX_TOKENS);
  if (!allTokens)
  {
    fprintf(stderr, "malloc failed");
    exit(EXIT_FAILURE);
  }
  int tokenCount = 0;

  EnviornmentVariableReplace envs[64];
  while (input[i])
  {
    short isDoubleQuotes = 0;
    short isSingleQuotes = 0;
    int envCount = 0;

    if (input[i] == ' ')
    {
      i++;
      continue;
    }

    if (input[i] == '"')
    {
      isDoubleQuotes = 1;
      i++;
    }
    else if (input[i] == '\'')
    {
      isSingleQuotes = 1;
      i++;
    }

    int tokenLength = 0;
    int start = i;
    bool isOperator = false;
    int memAllocLength = 0;

    while (input[i])
    {
      if (!isSingleQuotes && input[i] == '$')
      {
        int insertPosition = i; // temp copy the insert position
        i++;                    // skip $
        char variableName[256]; // for the key of enviornment variable

        int j = 0;
        while (input[i] && (isalnum(input[i]) || input[i] == '_'))
          variableName[j++] = input[i++];
        variableName[j] = '\0';

        char *envVal = myGetenv(variableName, env);
        if (envVal)
          envVal = strdup(envVal);

        envs[envCount].env = envVal;
        envs[envCount].varNameLength = i - insertPosition;
        envs[envCount].valLength = envVal ? strlen(envVal) : 0;
        tokenLength += envs[envCount].varNameLength;
        memAllocLength += envs[envCount].valLength;
        envCount++;
      }

      else if (!isDoubleQuotes && !isSingleQuotes && input[i] == ' ')
      {
        break;
      }

      else if (!isDoubleQuotes && !isSingleQuotes && (input[i] == ';' || input[i] == '>' || input[i] == '<'))
      {
        if (start == i)
        {
          isOperator = true;
          tokenLength++;
          i++;
          memAllocLength += 1;
        }
        break;
      }

      else if (!isDoubleQuotes && !isSingleQuotes && (input[i] == '|' || input[i] == '&'))
      {
        if (start == i)
        {
          if (input[i + 1] && input[i + 1] == input[i])
          {
            i++;
            tokenLength++;
            memAllocLength += 1;
          }
          memAllocLength += 1;
          isOperator = true;
          tokenLength++;
          i++;
        }
        break;
      }

      else if (input[i] == '"' && isDoubleQuotes) // end quotes
      {
        isDoubleQuotes = 2;
        i++;
        break;
      }

      else if (input[i] == '\'' && isSingleQuotes) // end quotes
      {
        isSingleQuotes = 2;
        i++;
        break;
      }

      else if (input[i] == '\\')
      {
        if (!input[i + 1])
        {
          printf("invalid input");
          exit(0);
        }

        memAllocLength += 1;
        if (isSingleQuotes)
        {
          memAllocLength += 1; // also considering \ value for ''
        }

        tokenLength += 2;
        i += 2; // skipping backslash
      }

      else // all general characters
      {
        i++;
        tokenLength++;
        memAllocLength += 1;
      }
    }

    int k = 0;
    int envidx = 0;
    if (!input[i] && (isDoubleQuotes == 1 || isSingleQuotes == 1))
    {
      fprintf(stderr, "wrong syntax expected a closing quote\n");
      exit(EXIT_FAILURE);
    }

    char *val = (char *)malloc(sizeof(char) * (memAllocLength + 1)); // +1 for \0
    if (!val)
    {
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }

    for (int j = 0; j < tokenLength; j++)
    {
      if (!isSingleQuotes && input[start + j] == '$')
      {
        EnviornmentVariableReplace curr = envs[envidx++];
        if (curr.env != NULL)
        {
          strcpy(&val[k], curr.env);
        }
        k += curr.valLength;
        val[k] = '\0';
        j += curr.varNameLength - 1; // +1 lenght in loop condition
        continue;
      }
      if (!isSingleQuotes && input[start + j] == '\\')
      {
        j++;
      }
      val[k++] = input[start + j];
    }

    val[k] = '\0';
    Token *token = (Token *)malloc(sizeof(Token));
    if (!token)
    {
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }
    token->token = val;
    token->isOperator = isOperator;
    allTokens[tokenCount++] = token;
  }
  allTokens[tokenCount] = NULL;
  return allTokens;
}

void splitCommands(Token **allTokens)
{
  int i = 0;
  int sepSemicolon = 0; // count all commands seperated by ;

  PipelineComponent *pc = NULL; // pipeline component
  Pipeline *p = NULL;           // pipeline
  Command *fc = NULL;           // full command

  for (int i = 0; allTokens[i]; i++)
  {
    if (isSemicolon(allTokens[i]))
    {
      sepSemicolon++;
    }
  }

  Commands *ac = createCommands();

  int tokeni = 0;
  while (allTokens[i])
  {
    Token *token = allTokens[i];

    if (isPipe(token))
    {
      if (!pc)
      {
        i++;
        fprintf(stderr, "Error near %s\n", token->token);
        exit(EXIT_FAILURE);
      }

      if (!p)
      {
        p = createPipeline();
      }

      push_PipelineComponent(p->components, pc);
      pc = NULL;
      i++;
      continue;
    }
    else if (isLogicalOp(token))
    {
      if (!pc)
      {
        i++;
        fprintf(stderr, "Error near %s\n", token->token);
        exit(EXIT_FAILURE);
      }

      if (!p)
      {
        p = createPipeline();
      }

      if (!fc)
      {
        fc = createCommand();
      }

      push_PipelineComponent(p->components, pc);
      push_Pipeline(fc->pipelines, p);

      if (strcmp(token->token, "&&") == 0)
      {
        p->separator = AND;
      }
      else if (strcmp(token->token, "||") == 0)
      {
        p->separator = OR;
      }

      pc = NULL;
      p = NULL;
      i++;
      continue;
    }
    else if (isSemicolon(token))
    {
      if (!pc)
      {
        i++;
        fprintf(stderr, "Error near %s\n", token->token);
        exit(EXIT_FAILURE);
      }

      if (!p)
      {
        p = createPipeline();
      }

      if (!fc)
      {
        fc = createCommand();
      }

      push_PipelineComponent(p->components, pc);
      push_Pipeline(fc->pipelines, p);
      push_Command(ac->commands, fc);

      p->separator = -1; // not real separator it just ;
      pc = NULL;
      p = NULL;
      fc = NULL;
      i++;
      continue;
    }

    if (!pc)
    {
      pc = createPipelineComponent();
    }

    if (isLt(token))
      pc->isLt = true;
    if (isGt(token))
      pc->isGt = true;

    push_Token(pc->tokens, token);
    i++;
  }

  if (i >= 1 && !isSemicolon(allTokens[i - 1]))
  {
    if (!pc)
    {
      fprintf(stderr, "Error: Invalid input near 'EOF'\n");
      exit(EXIT_FAILURE);
    }

    if (!p)
    {
      p = createPipeline();
    }

    if (!fc)
    {
      fc = createCommand();
    }

    push_PipelineComponent(p->components, pc);
    push_Pipeline(fc->pipelines, p);
    push_Command(ac->commands, fc);
  }

  for (int i = 0; i < ac->commands->size; i++)
  {
    Command *currCommand = ac->commands->data[i];

    for (int j = 0; j < currCommand->pipelines->size; j++)
    {
      Pipeline *currPipeline = currCommand->pipelines->data[j];
      for (int k = 0; k < currPipeline->components->size; k++)
      {
        PipelineComponent *currPipelineComp = currPipeline->components->data[k];
        for (int m = 0; m < currPipelineComp->tokens->size; m++)
        {
          printf("'%s' ", currPipelineComp->tokens->data[m]->token);
        }
        printf("\n-- Lt: %d Gt: %d\n", currPipelineComp->isLt, currPipelineComp->isGt);
      }
      printf("Separator: %d\n", currPipeline->separator);
      printf("----\n");
    }
    printf("--------\n");
  }
}

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

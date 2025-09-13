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

Commands *splitCommands(Token **allTokens)
{
  PipelineComponent *pc = NULL; // pipeline component
  Pipeline *p = NULL;           // pipeline
  Command *fc = NULL;           // full command

  Commands *ac = createCommands();

  int i = 0;
  while (allTokens[i])
  {
    Token *token = allTokens[i];

    if (isLt(token) || isGt(token))
    {
      Token *nextToken = allTokens[i + 1];
      if (!pc || nextToken == NULL || isDelimiter(nextToken))
      { // if first token or next commnad is not a file
        fprintf(stderr, "Error near %s\n", token->token);
        exit(EXIT_FAILURE);
      }
    }
    else if (isPipe(token))
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

  return ac;
}

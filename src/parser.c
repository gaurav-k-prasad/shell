#include "../headers/myshell.h"

VectorToken *getTokens(char *input, char **env)
{
  int i = 0;
  VectorToken *tokenVec = (VectorToken *)malloc(sizeof(VectorToken));
  if (initVecToken(tokenVec, 16) == -1)
  {
    free(tokenVec);
    return NULL;
  }

  if (!tokenVec)
  {
    return NULL;
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
          fprintf(stderr, "invalid input");
          for (int i = 0; i < envCount; i++)
            free(envs[i].env);
          freeVecToken(tokenVec);
          return NULL;
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
      fprintf(stderr, "error: expected a closing quote\n");
      freeVecToken(tokenVec);
      for (int i = 0; i < envCount; i++)
        free(envs[i].env);

      return NULL;
    }

    if (memAllocLength == 0) {
      for (int i = 0; i < envCount; i++)
        free(envs[i].env);
      continue;
    }
    
    char *val = (char *)malloc(sizeof(char) * (memAllocLength + 1)); // +1 for \0
    if (!val)
    {
      freeVecToken(tokenVec);
      for (int i = 0; i < envCount; i++)
        free(envs[i].env);

      return NULL;
    }

    for (int j = 0; j < tokenLength; j++)
    {
      if (!isSingleQuotes && input[start + j] == '$')
      {
        EnviornmentVariableReplace curr = envs[envidx++];
        if (curr.env != NULL)
        {
          strcpy(&val[k], curr.env);
          free(curr.env); // free env value allocated by myGetenv
        }
        k += curr.valLength;
        val[k] = '\0';
        j += curr.varNameLength - 1; // +1 length in loop condition
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
      free(val);
      freeVecToken(tokenVec);
      for (int i = 0; i < envCount; i++)
        free(envs[i].env);

      return NULL;
    }
    token->token = val;
    token->isOperator = isOperator;

    if (pushVecToken(tokenVec, token) == -1)
    {
      freeVecToken(tokenVec);
      free(val);
      for (int i = 0; i < envCount; i++)
        free(envs[i].env);

      return NULL;
    }
  }
  return tokenVec;
}

Commands *splitCommands(VectorToken *tokenVec)
{
  PipelineComponent *pc = NULL; // pipeline component
  Pipeline *p = NULL;           // pipeline
  Command *fc = NULL;           // full command

  Commands *ac = createCommands();
  if (!ac)
    goto errorHandler;

  Token **allTokens = tokenVec->data;

  int i = 0;
  while (allTokens[i])
  {
    Token *token = allTokens[i];

    if (isLt(token) || isGt(token))
    {
      Token *nextToken = allTokens[i + 1];
      if (!pc || nextToken == NULL || isDelimiter(nextToken))
      {
        // if first token or next commnad is not a file then throw an error
        fprintf(stderr, "error near %s\n", token->token);
        goto errorHandler;
      }
    }
    else if (isPipe(token))
    {
      if (!pc)
      {
        fprintf(stderr, "error near %s\n", token->token);
        goto errorHandler;
      }

      if (!p)
      {
        p = createPipeline();
        if (!p)
          goto errorHandler;
      }

      if (pushVecPipelineComponent(p->components, pc) == -1)
      {
        goto errorHandler;
      }
      pc = NULL;
      i++;
      continue;
    }
    else if (isLogicalOp(token))
    {
      if (!pc)
      {
        fprintf(stderr, "error near %s\n", token->token);
        goto errorHandler;
      }

      if (!p)
      {
        p = createPipeline();
        if (!p)
          goto errorHandler;
      }

      if (!fc)
      {
        fc = createCommand();
        if (!fc)
          goto errorHandler;
      }

      if (pushVecPipelineComponent(p->components, pc) == -1)
      {
        goto errorHandler;
      }
      if (pushVecPipeline(fc->pipelines, p) == -1)
      {
        goto errorHandler;
      }

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
        fprintf(stderr, "error near %s\n", token->token);
        goto errorHandler;
      }

      if (!p)
      {
        p = createPipeline();
        if (!p)
          goto errorHandler;
      }

      if (!fc)
      {
        fc = createCommand();
        if (!fc)
          goto errorHandler;
      }

      if (pushVecPipelineComponent(p->components, pc) == -1)
      {
        goto errorHandler;
      }
      if (pushVecPipeline(fc->pipelines, p) == -1)
      {
        goto errorHandler;
      }
      if (pushVecCommand(ac->commands, fc) == -1)
      {
        goto errorHandler;
      }

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
      if (!pc)
        goto errorHandler;
    }

    if (isLt(token))
      pc->isLt = true;
    if (isGt(token))
      pc->isGt = true;

    Token *newToken = createToken(token->token, token->isOperator);
    if (!newToken)
    {
      fprintf(stdout, "new token creation failed\n");
      goto errorHandler;
    }
    if (pushVecToken(pc->tokens, newToken) == -1)
    {
      goto errorHandler;
    }
    i++;
  }

  if (i >= 1 && !isSemicolon(allTokens[i - 1]))
  {
    if (!pc)
    {
      fprintf(stderr, "error: Invalid input near 'EOF'\n");
      goto errorHandler;
    }

    if (!p)
    {
      p = createPipeline();
      if (!p)
        goto errorHandler;
    }

    if (!fc)
    {
      fc = createCommand();
      if (!fc)
        goto errorHandler;
    }

    if (pushVecPipelineComponent(p->components, pc) == -1)
    {
      goto errorHandler;
    }
    if (pushVecPipeline(fc->pipelines, p) == -1)
    {
      goto errorHandler;
    }
    if (pushVecCommand(ac->commands, fc) == -1)
    {
      goto errorHandler;
    }
  }

  /* for (int i = 0; i < ac->commands->size; i++)
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
  } */

  return ac;

errorHandler:
  if (pc)
    freePipelineComponent(pc);
  if (p)
    freePipeline(p);
  if (fc)
    freeCommand(fc);
  freeCommands(ac);

  return NULL;
}

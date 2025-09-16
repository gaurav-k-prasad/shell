#include "../headers/myshell.h"

VectorToken *getTokensDeprecated(char *input, char **env)
{
  int i = 0;
  VectorToken *tokenVec = (VectorToken *)malloc(sizeof(VectorToken));
  if (!tokenVec)
  {
    return NULL;
  }
  if (initVecToken(tokenVec, 16) == -1)
  {
    free(tokenVec);
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
          if (input[i] && input[i + 1] == input[i])
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

    if (memAllocLength == 0)
    {
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

VectorToken *getTokens(char *input, char **env)
{
  int i = 0;
  VectorToken *tokenVec = (VectorToken *)malloc(sizeof(VectorToken));
  if (!tokenVec)
  {
    return NULL;
  }
  if (initVecToken(tokenVec, 16) == -1)
  {
    free(tokenVec);
    return NULL;
  }

  Token *currToken = createToken(NULL, 0, false, 1);
  if (!currToken)
    goto errorHandle;

  bool isDoubleQuotes = false;
  bool isSingleQuotes = false;
  bool isOperator = false;
  bool isFlag = false;
  bool isTokenEnd = false;
  while (true)
  {
    if (isTokenEnd || input[i] == '\0')
    {
      if (currToken->len > 0)
      {
        currToken->isOperator = isOperator;

        Token *expandedToken = expandToken(currToken, isOperator, isDoubleQuotes, isSingleQuotes, env);
        if (!expandedToken)
          goto errorHandle;

        if (expandedToken->len > 0 && pushVecToken(tokenVec, expandedToken) == -1)
        {
          freeToken(expandedToken);
          goto errorHandle;
        }
        freeToken(currToken); // currToken not needed as replaced by expanded tokens
        currToken = createToken(NULL, 0, false, 1);
        if (!currToken)
          goto errorHandle;
        isDoubleQuotes = false;
        isSingleQuotes = false;
        isOperator = false;
        isFlag = false;
        isTokenEnd = false;
      }

      if (input[i] == '\0')
        break;
    }
    if (input[i] == ' ')
    {
      isTokenEnd = true;
      i++;
    }
    else if (input[i] == '"' || input[i] == '\'')
    {
      if (isFlag)
      {
        isTokenEnd = true;
        continue;
      }
      char quote = input[i];

      i++; // skip " or '
      while (input[i] && input[i] != quote)
      {
        if (input[i] == '\\')
        {
          if (insertInTokenChar(currToken, input[i++]) == -1)
          {
            goto errorHandle;
          }
          if (!input[i])
          {
            fprintf(stderr, "syntax error\n");
            goto errorHandle;
          }
        }

        if (insertInTokenChar(currToken, input[i++]) == -1)
        {
          goto errorHandle;
        }
      }
      if (!input[i])
      {
        fprintf(stderr, "syntax error expected a %c\n", quote);
        goto errorHandle;
      }
      if (quote == '"')
        isDoubleQuotes = true;
      else
        isSingleQuotes = true;

      i++; // skip " or '
      isTokenEnd = false;
    }
    else if (input[i] == ';' || input[i] == '<')
    {
      if (currToken->len > 0) // flush the token
      {
        isTokenEnd = true;
        continue;
      }
      if (insertInTokenChar(currToken, input[i++]) == -1)
      {
        goto errorHandle;
      }
      isOperator = true;
      isTokenEnd = true;
    }
    else if (input[i] == '>' || input[i] == '&' || input[i] == '|')
    {
      if (currToken->len > 0) // flush the token
      {
        isTokenEnd = true;
        continue;
      }
      if (insertInTokenChar(currToken, input[i++]) == -1)
      {
        goto errorHandle;
      }
      if (input[i] && input[i] == input[i - 1])
      {
        if (insertInTokenChar(currToken, input[i++]) == -1)
        {
          goto errorHandle;
        }
      }
      isOperator = true;
      isTokenEnd = true;
    }
    else if (input[i] == '\\' && !input[i + 1])
    {
      fprintf(stderr, "syntax error near \\\n");
      goto errorHandle;
    }
    else // all general characters
    {
      if (input[i] == '\\')
      {
        if (insertInTokenChar(currToken, input[i++]) == -1)
        {
          goto errorHandle;
        }
        if (!input[i])
        {
          fprintf(stderr, "sytnax error expected a '\n");
          goto errorHandle;
        }
      }
      else if (input[i] == '-')
      {
        isFlag = true;
      }

      if (insertInTokenChar(currToken, input[i++]) == -1)
      {
        goto errorHandle;
      }
      isTokenEnd = false;
    }
  }

  freeToken(currToken); // at the end redundant token will be created
  return tokenVec;

errorHandle:
  freeVecToken(tokenVec);
  freeToken(currToken);
  return NULL;
}

Token *expandToken(Token *token, bool isOperator, bool isDQuotes, bool isSQuotes, char **env)
{

  if (!token)
    return NULL;

  char *input = token->token;
  Token *expandedToken;

  if (isOperator || isSQuotes) // if it's an operator or is in single quotes then no need for expansion
  {
    int len = myStrlen(input);
    expandedToken = createToken(input, len, isOperator, len);
    if (!expandedToken)
      goto errorHandle;

    return expandedToken;
  }
  else
  {
    expandedToken = createToken(NULL, 0, false, 1);
  }

  int i = 0;

  while (input[i])
  {
    if (input[i] == '\\')
    {
      i++;
      if (insertInTokenChar(expandedToken, input[i++]) == -1)
      {
        goto errorHandle;
      }
    }
    else if (input[i] == '$') // find the enviornment variable related to the given token
    {
      i++;                           // skip $
      char *envKeyStart = &input[i]; // starting pointer of enviornment variable key

      while (input[i] && isalnum(input[i]) || input[i] == '_') // find the end point of enviornment variable name
        i++;

      char lastChar = input[i]; // store the last character so that we can replace it back
      input[i] = '\0';          // after finding the enviornment variable we can subsitute back the input[i] = lastChar

      char *envVal = myGetenv(envKeyStart, env);
      if (insertInTokenStr(expandedToken, envVal) == -1)
      {
        goto errorHandle;
      }

      input[i] = lastChar;
      free(envVal);
    }
    else
    {
      // general characters
      if (insertInTokenChar(expandedToken, input[i++]) == -1)
      {
        goto errorHandle;
      }
    }
  }

  return expandedToken;

errorHandle:
  freeToken(expandedToken);

  return NULL;
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

    int copyTokenLen = myStrlen(token->token);
    Token *newToken = createToken(token->token, copyTokenLen, token->isOperator, copyTokenLen);
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

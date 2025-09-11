#include "declare.c"

Token ** getTokens(char *input) {
  int i = 0;
  Token **allTokens = (Token **)malloc(sizeof(Token *) * MAX_TOKENS);
  int tokenCount = 0;
  
  while (input[i])
  {
    short isDoubleQuotes = 0;
    short isSingleQuotes = 0;
    int envCount = 0;
    EnviornmentVariableReplace envs[64];

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

        char *envVal = getenv(variableName);
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
    if (!input[i] && (isDoubleQuotes == 1 || isSingleQuotes == 1)) {
      fprintf(stderr, "wrong syntax expected a closing quote\n");
      exit(EXIT_FAILURE);
    }

    char *val = (char *)malloc(sizeof(char) * (memAllocLength + 1)); // +1 for \0

    for (int j = 0; j < tokenLength; j++)
    {
      if (!isSingleQuotes && input[start + j] == '$')
      {
        EnviornmentVariableReplace curr = envs[envidx++];
        if (curr.env != NULL) {
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
    token->token = val;
    token->isOperator = isOperator;
    allTokens[tokenCount++] = token;
  }
  allTokens[tokenCount] = NULL;
  return allTokens;
}
#include "../headers/myshell.h"

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
      freeTokens(tokens);
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

// Free allocated memory
void freeTokens(char **tokens)
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
#include "myshell.h"

char **parseInput(char *input)
{
  size_t buffer_size = MAX_INPUT_BUF;
  char **tokens = (char **)malloc(buffer_size * sizeof(char *));
  char *token = NULL;
  size_t position = 0;
  size_t tokenLength = 0;

  if (!tokens)
  {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }

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

    tokens[position] = malloc((tokenLength + 1) * sizeof(char)); // +1 for EOF

    if (!tokens[position])
    {
      perror("malloc failed");
      exit(EXIT_FAILURE);
    }

    for (size_t j = 0; j < tokenLength; j++)
    {
      tokens[position][j] = token[j];
    }
    tokens[position][tokenLength] = '\0'; // null terminate the token
    position++;
    tokenLength = 0; // reset token length
    token = NULL;
  }

  tokens[position] = NULL; // Terminate array with null
  return tokens;
}

// Free allocated memory
void freeTokens(char **tokens) {
  for (int i = 0; tokens[i]; i++) {
    free(tokens[i]);
  }

  free(tokens);
}
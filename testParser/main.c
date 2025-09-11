#include "declare.c"

void splitCommands(Token **allTokens);

bool isSemicolon(Token *input)
{
  return (input->isOperator && strcmp(input->token, ";") == 0);
}

bool isLogicalOp(Token *input)
{
  return (input->isOperator && (strcmp(input->token, "||") == 0 || strcmp("&&", input->token) == 0));
}

bool isPipe(Token *input)
{
  return (input->isOperator && strcmp(input->token, "|") == 0);
}

bool isLt(Token *input)
{
  return (input->isOperator && strcmp(input->token, "<") == 0);
}

bool isGt(Token *input)
{
  return (input->isOperator && strcmp(input->token, ">") == 0);
}

PipelineComponent *createPipelineComponent()
{
  PipelineComponent *pc = (PipelineComponent *)malloc(sizeof(PipelineComponent));
  pc->isGt = 0;
  pc->isLt = 0;
  pc->tokens = (Token **)malloc(sizeof(Token *) * 64); // +1 for NULL
  pc->n = 0;

  return pc;
}

Pipeline *createPipeline()
{
  Pipeline *p = (Pipeline *)malloc(sizeof(Pipeline));
  p->components = (PipelineComponent **)malloc(sizeof(PipelineComponent *) * 64);
  p->n = 0;

  return p;
}

Command *createFullCommand()
{
  Command *fc = (Command *)malloc(sizeof(Command));
  fc->pipelines = (Pipeline **)malloc(sizeof(Pipeline *) * 64);
  fc->separator = -1;
  fc->n = 0;

  return fc;
}

int main(int argc, char *argv[], char *env[])
{

  char *input = NULL;
  size_t size = 0;
  getline(&input, &size, stdin);
  input[strcspn(input, "\n")] = '\0';

  // ls "g p" -> "ls", "g p"
  Token **allTokens = getTokens(input);
  for (int i = 0; allTokens[i]; i++)
  {
    Token *token = allTokens[i];
    printf("token: '%s'\nop %d\n-------\n", token->token, token->isOperator);
    // free(token);
  }

  splitCommands(allTokens);

  free(allTokens);
  free(input);
  return 0;
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

  Commands *ac = (Commands *)malloc(sizeof(Commands)); // all commands
  // number of commands can be semicolons+1, +1 for NULL
  /*
  echo a;           => +1 not needed
  echo a; echo b    => +1 needed
  */
  ac->commands = (Command **)malloc(sizeof(Command *) * (sepSemicolon + 1 + 1));
  ac->n = 0;

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

      p->components[p->n++] = pc;
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
        fc = createFullCommand();
      }

      p->components[p->n++] = pc;
      fc->pipelines[fc->n++] = p;

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
        fc = createFullCommand();
      }

      p->components[p->n++] = pc;
      fc->pipelines[fc->n++] = p;
      ac->commands[ac->n++] = fc;

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

    pc->tokens[pc->n++] = token;
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
      fc = createFullCommand();
    }

    p->components[p->n++] = pc;
    fc->pipelines[fc->n++] = p;
    ac->commands[ac->n++] = fc;
  }

  ac->commands[ac->n] = NULL;

  for (int i = 0; i < ac->n; i++)
  {
    for (int j = 0; j < ac->commands[i]->n; j++)
    {
      Pipeline curr = *(ac->commands[i]->pipelines[j]);
      for (int k = 0; k < curr.n; k++)
      {
        PipelineComponent c = *(curr.components[k]);
        for (int m = 0; m < c.n; m++)
        {
          printf("%s ", c.tokens[m]->token);
        }
        printf("\n--\n");
      }
      printf("----\n");
    }
    printf("--------\n");
  }
}
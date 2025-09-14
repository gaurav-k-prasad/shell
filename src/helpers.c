#include "../headers/myshell.h"

int myStrcmp(const char *a, const char *b)
{

  while (*a && (*a == *b))
  {
    a++;
    b++;
  }

  return *(unsigned char *)a - *(unsigned char *)b;
}

int myStrnicmp(const char *a, const char *b, int n)
{
  int i = 0;
  while (i < n && *a && (tolower(*a) == tolower(*b)))
  {
    a++;
    b++;
    i++;
  }

  if (i == n)
  {
    return 0;
  }
  return *(unsigned char *)a - *(unsigned char *)b;
}

int myStrlen(const char *str)
{
  int len = 0;

  while (*str)
  {
    len++;
    str++;
  }

  return len;
}

char *myGetenv(const char *name, char **env)
{
  if (name == NULL || env == NULL)
  {
    return NULL;
  }

  size_t nameLen = myStrlen(name);

  for (int i = 0; env[i]; i++)
  {
    if (myStrnicmp(env[i], name, nameLen) == 0 && env[i][nameLen] == '=')
    {
      int envValLen = myStrlen(env[i] + nameLen + 1);
      char *buff = (char *)malloc(sizeof(char) * (envValLen + 1));

      if (!buff)
      {
        perror("malloc");
        exit(EXIT_FAILURE);
      }

      strcpy(buff, env[i] + nameLen + 1);
      buff[envValLen] = '\0';
      return buff;
    }
  }
  return NULL;
}

void myStrcpy(char *str1, const char *str2)
{
  size_t strLen = myStrlen(str2);

  for (int i = 0; i < strLen; i++)
  {
    str1[i] = str2[i];
  }
  str1[strLen] = '\0';
}

char *myStrdup(const char *str)
{
  if (str == NULL)
    return NULL;
  size_t _strlen = myStrlen(str);
  char *dup = (char *)malloc(sizeof(char) * (_strlen + 1)); // caller has to free
  if (!dup)
  {
    perror("malloc");
    return NULL;
  }

  myStrcpy(dup, str);
  return dup;
}

char *myStrchr(const char *str, const char delimiter)
{
  while (*str)
  {
    if (*str == delimiter)
    {
      return (char *)str;
    }
    str++;
  }

  return NULL;
}

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

bool isBuiltin(char *command)
{
  if (myStrcmp(command, "export") == 0 ||
      myStrcmp(command, "unset") == 0 ||
      myStrcmp(command, "cd") == 0 ||
      myStrcmp(command, "exit") == 0 ||
      myStrcmp(command, "quit") == 0)
  {
    return true;
  }

  return false;
}

int handleBuiltin(char **args, char ***env, char *initialDirectory)
{
  if (myStrcmp(args[0], "export") == 0)
  {
    *env = commandExport(args, *env);
    if (*env == NULL)
      return 1;
  }
  else if (myStrcmp(args[0], "unset") == 0)
  {
    *env = commandUnset(args, *env);
    if (*env == NULL)
      return 1;
  }
  else if (myStrcmp(args[0], "cd") == 0)
  {
    return commandCd(args, initialDirectory);
  }
  else if (myStrcmp(args[0], "exit") == 0 || myStrcmp(args[0], "quit") == 0)
  {
    exit(EXIT_SUCCESS);
  }
}

bool isMyImplementedBuiltin(char *command)
{
  if (myStrcmp(command, "echo") == 0 ||
      myStrcmp(command, "pwd") == 0 ||
      myStrcmp(command, "env") == 0 ||
      myStrcmp(command, "which") == 0)
  {
    return true;
  }

  return false;
}

int handleMyImplementedBuiltin(char **args, char ***env, char *initialDirectory)
{
  if (myStrcmp(args[0], "echo") == 0)
  {
    return commandEcho(args, *env);
  }
  else if (myStrcmp(args[0], "pwd") == 0)
  {
    return commandPwd();
  }
  else if (myStrcmp(args[0], "env") == 0)
  {
    return commandEnv(*env);
  }
  else if (myStrcmp(args[0], "which") == 0)
  {
    return commandWhich(args, *env);
  }
}

void closePipes(int fds[][2], int n)
{
  for (int i = 0; i < n; i++)
  {
    close(fds[i][0]);
    close(fds[i][1]);
  }
}

void findInOutFileAndCommandEnd(PipelineComponent *pc, char **infile, char **outfile, int *commandEnd)
{
  Token **tokens = pc->tokens->data;

  if (pc->isLt)
  {
    int i = 0;
    while (i < pc->tokens->size)
    {
      if (isLt(tokens[i]))
      {
        *infile = tokens[i + 1]->token;
        *commandEnd = MIN(*commandEnd, i);
      }
      i++;
    }
  }

  if (pc->isGt)
  {
    int i = 0;
    while (i < pc->tokens->size)
    {
      if (isGt(tokens[i]))
      {
        *outfile = tokens[i + 1]->token;
        *commandEnd = MIN(*commandEnd, i);
      }
      i++;
    }
  }
}

Token *createToken(char *str, bool isOperator)
{
  Token *tok = (Token *)malloc(sizeof(Token));
  if (!tok)
  {
    perror("malloc");
    return NULL;
  }

  tok->isOperator = isOperator;
  if (str)
  {
    int len = myStrlen(str);
    char *newStr = (char *)malloc(sizeof(char) * (len + 1)); // +1 for \0
    if (!newStr)
    {
      perror("malloc");
      free(tok);
      return NULL;
    }
    myStrcpy(newStr, str);
    tok->token = newStr;
  }
  else
  {
    tok->token = NULL;
  }

  return tok;
}

PipelineComponent *createPipelineComponent()
{
  PipelineComponent *pc = (PipelineComponent *)malloc(sizeof(PipelineComponent));
  if (!pc)
  {
    return NULL;
  }

  VectorToken *vt = (VectorToken *)malloc(sizeof(VectorToken));
  if (!vt)
  {
    free(pc);
    return NULL;
  }
  initVecToken(vt, 4);

  pc->isGt = 0;
  pc->isLt = 0;
  pc->tokens = vt; // +1 for NULL

  return pc;
}

Pipeline *createPipeline()
{
  Pipeline *p = (Pipeline *)malloc(sizeof(Pipeline));
  if (!p)
  {
    return NULL;
  }

  VectorPipelineComponent *vpc = (VectorPipelineComponent *)malloc(sizeof(VectorPipelineComponent));
  if (!vpc)
  {
    free(p);
    return NULL;
  }
  initVecPipelineComponent(vpc, 4);

  p->components = vpc;
  p->separator = -1; // Pipeline separator not defined yet

  return p;
}

Command *createCommand()
{
  Command *fc = (Command *)malloc(sizeof(Command));
  if (!fc)
  {
    return NULL;
  }

  VectorPipeline *vp = (VectorPipeline *)malloc(sizeof(VectorPipeline));
  if (!vp)
  {
    free(fc);
    return NULL;
  }
  initVecPipeline(vp, 4);
  fc->pipelines = vp;
  return fc;
}

Commands *createCommands()
{
  Commands *cmds = (Commands *)malloc(sizeof(Commands));
  if (!cmds)
  {
    return NULL;
  }

  VectorCommand *vc = (VectorCommand *)malloc(sizeof(VectorCommand));
  if (!vc)
  {
    free(cmds);
    return NULL;
  }
  initVecCommand(vc, 4);
  cmds->commands = vc;
  return cmds;
}

bool isDelimiter(Token *token)
{
  return (isPipe(token) || isLogicalOp(token) || isSemicolon(token) || (isLt(token) && token->isOperator) || (isGt(token) && token->isOperator));
}

void freeToken(Token *token)
{
  if (token)
  {
    if (token->token)
    {
      free(token->token);
    }
    free(token);
  }
}

void freeVecToken(VectorToken *vec)
{
  if (!vec)
    return;

  for (int i = 0; i < vec->size; i++)
  {
    freeToken(vec->data[i]);
  }
  free(vec);
}

void freePipelineComponent(PipelineComponent *pc)
{
  if (pc)
  {
    if (pc->tokens)
    {
      freeVecToken(pc->tokens);
    }
    free(pc);
  }
}

void freeVecPipelineComponent(VectorPipelineComponent *vec)
{
  if (!vec)
    return;

  for (int i = 0; i < vec->size; i++)
  {
    freePipelineComponent(vec->data[i]);
  }
  free(vec);
}

void freePipeline(Pipeline *p)
{
  if (p)
  {
    if (p->components)
    {
      freeVecPipelineComponent(p->components);
    }
    free(p);
  }
}

void freeVecPipeline(VectorPipeline *vec)
{
  if (!vec)
    return;

  for (int i = 0; i < vec->size; i++)
  {
    freePipeline(vec->data[i]);
  }
  free(vec);
}

void freeCommand(Command *c)
{
  if (c)
  {
    if (c->pipelines)
    {
      freeVecPipeline(c->pipelines);
    }
    free(c);
  }
}

void freeVecCommand(VectorCommand *vec)
{
  if (!vec)
    return;

  for (int i = 0; i < vec->size; i++)
  {
    freeCommand(vec->data[i]);
  }
  free(vec);
}

void freeCommands(Commands *cs)
{
  if (!cs)
  {
    return;
  }

  if (cs->commands)
  {
    freeVecCommand(cs->commands);
  }
  free(cs);
}
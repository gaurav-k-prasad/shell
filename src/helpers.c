#include "../headers/myshell.h"

void handleSignal(int sig)
{
  write(STDOUT_FILENO, "\n", 1);
}

void printShellStart(char **env, char *userName)
{
  char cwd[1024], line[2048];
  getcwd(cwd, sizeof(cwd));

  if (userName)
    sprintf(line, "\033[36m@%s\033[0m \033[32m[%s]> \033[0m", userName, cwd);
  else
    sprintf(line, "\033[32m[%s]> \033[0m", cwd); // ANSI format coloring

  write(STDOUT_FILENO, line, myStrlen(line));
}

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
        return NULL;
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
    return commandExport(args, env);
  }
  else if (myStrcmp(args[0], "unset") == 0)
  {
    return commandUnset(args, env);
  }
  else if (myStrcmp(args[0], "cd") == 0)
  {
    return commandCd(args, initialDirectory);
  }
  else if (myStrcmp(args[0], "exit") == 0 || myStrcmp(args[0], "quit") == 0)
  {
    exit(EXIT_SUCCESS);
  }

  return 0;
}

void killPids(int start, int end, int pids[])
{
  for (int j = start; j < end; j++)
  {
    printf("pid: %d", pids[j]);
    if (pids[j] > 0)
    {
      fflush(stdout);
      kill(pids[j], SIGKILL);
    }
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

Token *createToken(char *str, int len, bool isOperator, int capacity)
{
  if (capacity < len || capacity < 1)
  {
    return NULL;
  }

  Token *tok = (Token *)malloc(sizeof(Token));
  if (!tok)
  {
    return NULL;
  }

  char *newStr = (char *)malloc(sizeof(char) * (capacity + 1)); // +1 for \0
  if (!newStr)
  {
    free(tok);
    return NULL;
  }
  if (str)
    strncpy(newStr, str, len);

  newStr[len] = '\0';
  tok->token = newStr;
  tok->isOperator = isOperator;
  tok->capacity = capacity;
  tok->len = len;
  return tok;
}

int insertInTokenChar(Token *token, char c)
{
  if (token->len == token->capacity)
  {
    char *newStr = (char *)realloc(token->token, sizeof(char) * (token->capacity * 2 + 1));
    if (!newStr)
    {
      return -1;
    }
    token->token = newStr;
    token->capacity *= 2;
  }

  token->token[token->len++] = c;
  token->token[token->len] = '\0';
  return 0;
}

int insertInTokenStr(Token *token, char *str)
{
  if (!str) // nothing to insert
    return 0;

  int i = 0;
  while (str[i])
  {
    if (insertInTokenChar(token, str[i++]) == -1)
    {
      return -1;
    }
  }

  return 0;
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

  if (initVecToken(vt, 4) == -1)
  {
    free(pc);
    free(vt);
    return NULL;
  }

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
  if (initVecPipelineComponent(vpc, 4) == -1)
  {
    free(p);
    free(vpc);
    return NULL;
  }

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

  if (initVecPipeline(vp, 4))
  {
    free(fc);
    free(vp);
    return NULL;
  }
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
  if (initVecCommand(vc, 4))
  {
    free(cmds);
    free(vc);
    return NULL;
  }
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
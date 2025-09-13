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
        fprintf(stderr, "malloc failed\n");
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
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
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

void handleBuiltin(char **args, char ***env, char *initialDirectory)
{
  if (myStrcmp(args[0], "export") == 0)
  {
    *env = commandExport(args, *env);
  }
  else if (myStrcmp(args[0], "unset") == 0)
  {
    *env = commandUnset(args, *env);
  }
  else if (myStrcmp(args[0], "cd") == 0)
  {
    commandCd(args, initialDirectory);
  }
  else if (myStrcmp(args[0], "exit") == 0 || myStrcmp(args[0], "quit") == 0)
  {
    exit(EXIT_SUCCESS);
  }
}

bool isMyImplementedBulitin(char *command)
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

void handleMyImplementedBulitin(char **args, char ***env, char *initialDirectory)
{
  if (myStrcmp(args[0], "echo") == 0)
  {
    commandEcho(args, *env);
  }
  else if (myStrcmp(args[0], "pwd") == 0)
  {
    commandPwd();
  }
  else if (myStrcmp(args[0], "env") == 0)
  {
    commandEnv(*env);
  }
  else if (myStrcmp(args[0], "which") == 0)
  {
    commandWhich(args, *env);
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

void findInOutFileAndCommandEnd(PipelineComponent *pc, int tokensLen, Token **tokens, char **infile, char **outfile, int *commandEnd)
{
  if (pc->isLt)
  {
    int i = 0;
    while (i < tokensLen)
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
    while (i < tokensLen)
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

PipelineComponent *createPipelineComponent()
{
  PipelineComponent *pc = (PipelineComponent *)malloc(sizeof(PipelineComponent));
  if (!pc)
  {
    return NULL;
  }

  Vector_Token *vt = (Vector_Token *)malloc(sizeof(Vector_Token));
  init_Token(vt, 4);

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

  Vector_PipelineComponent *vpc = (Vector_PipelineComponent *)malloc(sizeof(Vector_PipelineComponent));
  init_PipelineComponent(vpc, 4);

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

  Vector_Pipeline *vp = (Vector_Pipeline *)malloc(sizeof(Vector_Pipeline));
  init_Pipeline(vp, 4);
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

  Vector_Command *vc = (Vector_Command *)malloc(sizeof(Vector_Command));
  init_Command(vc, 4);
  cmds->commands = vc;
  return cmds;
}

bool isDelimiter(Token *token)
{
  return (isPipe(token) || isLogicalOp(token) || isSemicolon(token) || (isLt(token) && token->isOperator) || (isGt(token) && token->isOperator));
}
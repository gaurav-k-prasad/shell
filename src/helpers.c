#include "../headers/myshell.h"

extern volatile int termCols;
extern int whichSignal;

void handleSignal(int sig)
{
  if (sig == SIGINT)
  {
    write(STDOUT_FILENO, "^C\n", 3);
    whichSignal = SIGINT;
  }
  else if (sig == SIGCHLD)
  {
    waitpid(-1, NULL, WNOHANG); // reap the child process
    whichSignal = SIGCHLD;
  }
  else if (sig == SIGWINCH)
  {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    // prevTermCols = termCols;
    termCols = -w.ws_col; // negative indicates that it needs to be handled
    whichSignal = SIGWINCH;
  }
}

void printShellStart(char **env, char *userName)
{
  char cwd[1024], line[2048];
  getcwd(cwd, sizeof(cwd));

  if (userName)
    sprintf(line, "\033[36m@%s\033[0m \033[32m[%s]> \033[0m\n%s", userName, cwd, PROMPT);
  else
    sprintf(line, "\033[32m[%s]> \033[0m\n%s", cwd, PROMPT); // ANSI format coloring

  write(STDOUT_FILENO, line, myStrlen(line));
}

void clearText(int len, int termCols)
{
  int numberOfLinesToClear = ((len + myStrlen(PROMPT)) / termCols) + 1;

  for (int i = 0; i < numberOfLinesToClear - 1; i++)
  {
    printf(CLEAR_LINE);
    printf(UP);
    fflush(stdout);
  }
  printf("\033[%ldG\033[K", strlen(PROMPT) + 1); // do not delete the PROMPT length
  fflush(stdout);
}

void printPrompt(char *prompt)
{
  printf("%s", prompt);
  fflush(stdout);
}

void seek(int currPosition, int seekPosition, int termCols)
{
  int absCurrLen = (currPosition + myStrlen(PROMPT));
  int absSeekLen = (seekPosition + myStrlen(PROMPT));
  int currPosLineNumber = absCurrLen / termCols;
  int seekPosLineNumber = absSeekLen / termCols;
  int nRights = absSeekLen % termCols;

  if (seekPosition > currPosition)
  {
    for (int i = currPosLineNumber; i < seekPosLineNumber; i++)
    {
      printf(DOWN);
    }
    printf("\r");
    printf("\033[%dG", nRights + 1);
  }
  else if (seekPosition < currPosition)
  {
    for (int i = currPosLineNumber; i > seekPosLineNumber; i--)
    {
      printf(UP);
    }
    printf("\r");
    printf("\033[%dG", nRights + 1);
  }
  fflush(stdout);
}

void writeBufferOnTerminal(char *buffer, int length, int termCols)
{
  printf("\033[33m"); // yellow color
  fflush(stdout);
  write(STDOUT_FILENO, buffer, length);
  printf("\033[0m"); // reset color
  fflush(stdout);
  if ((myStrlen(PROMPT) + length) % termCols == 0)
  {
    printf("\n\r");
    fflush(stdout);
  }
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

bool isBackground(Token *input)
{
  return (input->isOperator && strcmp(input->token, "&") == 0);
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
      myStrcmp(command, "quit") == 0 ||
      myStrcmp(command, "source") == 0)
  {
    return true;
  }

  return false;
}

bool isAppend(Token *input)
{
  return (input->isOperator && strcmp(input->token, ">>") == 0);
}

int writeNHistoryInfoToFile(const char *file, int n, ForgettingDoublyLinkedList *history)
{
  FILE *historyInfo = fopen(file, "w");
  if (!historyInfo)
    return -1;
  ListNode *currHistory = history->tail;
  int historyWrittenCount = 0;
  while (currHistory && historyWrittenCount < 5)
  {
    fprintf(historyInfo, "%s\n", currHistory->command);
    currHistory = currHistory->prev;
    historyWrittenCount++;
  }
  fclose(historyInfo);
  return 0;
}

int writePlatformInfoToFile(const char *file)
{
  FILE *platformInfo = fopen(file, "w");
  if (!platformInfo)
    return -1;

  struct utsname sysinfo;
  char cwd[PATH_MAX];

  if (uname(&sysinfo) == 0)
  {
    fprintf(platformInfo, "OS: %s %s %s (%s)\n",
            sysinfo.sysname,
            sysinfo.release,
            sysinfo.version,
            sysinfo.machine);
  }

  if (getcwd(cwd, sizeof(cwd)) != NULL)
  {
    fprintf(platformInfo, "CWD: %s\n", cwd);
  }
  fclose(platformInfo);
  return 0;
}

int writeErrorInfoToFile(const char *file, const char *err)
{
  FILE *errorInfo = fopen(file, "w");
  if (!errorInfo)
    return -1;

  fprintf(errorInfo, "%s", err);

  fclose(errorInfo);
  return 0;
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
  else if (myStrcmp(args[0], "source") == 0)
  {
    return commandSource(args, env, initialDirectory);
  }

  return -1;
}

void killPids(int start, int end, int pids[])
{
  for (int j = start; j < end; j++)
  {
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
      myStrcmp(command, "which") == 0 ||
      myStrcmp(command, "ai") == 0)
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
  else if (myStrcmp(args[0], "ai") == 0)
  {
    return commandAI(args);
  }
  return -1;
}

void closePipes(int fds[][2], int n)
{
  for (int i = 0; i < n; i++)
  {
    close(fds[i][0]);
    close(fds[i][1]);
  }
}

bool findInOutFileAndCommandEnd(PipelineComponent *pc, char **infile, char **outfile, int *commandEnd)
{
  Token **tokens = pc->tokens->data;
  bool isAppendOutputFile = false;

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
        isAppendOutputFile = false;
      }
      else if (isAppend(tokens[i]))
      {
        *outfile = tokens[i + 1]->token;
        *commandEnd = MIN(*commandEnd, i);
        isAppendOutputFile = true;
      }
      i++;
    }
  }

  return isAppendOutputFile;
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
  fc->isBackground = false;
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

AICommands *createAICommand()
{
  AICommands *aiCommands = (AICommands *)malloc(sizeof(AICommands));
  if (!aiCommands)
  {
    return NULL;
  }

  aiCommands->commands = malloc(sizeof(char *) * MAX_AI_COMMANDS);
  if (!aiCommands->commands)
  {
    free(aiCommands);
    return NULL;
  }
  aiCommands->warning = NULL;
  aiCommands->explanation = NULL;
  aiCommands->commandsCount = 0;

  return aiCommands;
}

AIQuestions *createAIQuestions()
{
  AIQuestions *aiQuestions = (AIQuestions *)malloc(sizeof(AIQuestions));
  if (!aiQuestions)
  {
    return NULL;
  }

  aiQuestions->questions = malloc(sizeof(char *) * MAX_AI_QUESTIONS);
  if (!aiQuestions->questions)
  {
    free(aiQuestions);
    return NULL;
  }
  aiQuestions->explanation = NULL;
  aiQuestions->questionsCount = 0;

  return aiQuestions;
}

bool isDelimiter(Token *token)
{
  return (isPipe(token) || isLogicalOp(token) || isSemicolon(token) || (isLt(token) && token->isOperator) || (isGt(token) && token->isOperator) || (isAppend(token) && token->isOperator));
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

void freeAICommands(AICommands *commands)
{
  if (!commands)
    return;

  free(commands->warning);
  free(commands->explanation);
  for (int i = 0; i < commands->commandsCount; i++)
    free(commands->commands[i]);
  free(commands->commands);
}

void freeAIQuestions(AIQuestions *questions)
{
  if (!questions)
    return;

  free(questions->explanation);
  for (int i = 0; i < questions->questionsCount; i++)
    free(questions->questions[i]);
  free(questions->questions);
}
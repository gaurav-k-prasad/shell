#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_INPUT_BUF 1024

char **parseInput(char *input);
void freeTokens(char **tokens);
int shellBuilts(char **args, char **env, char *initialDirectory);
char *getFullPathOfWhich(char *command, char **env);

// Helpers
int myStrcmp(const char *a, const char *b);
char* myGetenv(const char *name, char **env);
int myStrlen(const char *str);
int myStrnicmp(const char *a, const char *b, int n);

// Built in function implementation
int commandCd(char **args, char *initialDirectory);
int commandPwd();
int commandEcho(char **args, char **env);
int commandEnv(char **env);
int commandWhich(char **args, char **env);

char **commandSetenv(char **args, char **env);
char **commandUnsetenv(char **args, char **env);
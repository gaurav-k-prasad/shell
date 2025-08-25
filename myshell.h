#ifndef MYSHELL_H
#define MYSHELL_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#endif

#ifdef _WIN32
#define SEP ";"
#define PATH_SEPARATOR "\\"
#else
#define SEP ":"
#define PATH_SEPARATOR "/"
#endif

#define MAX_INPUT_BUF 1024

char **parseInput(char *input);
void freeTokens(char **tokens);
void freeEnv(char **env);
int shellBuilts(char **args, char **env, char *initialDirectory);
char *getFullPathOfWhich(char *command, char **env);
char **cloneEnv(char **env);

// Executor
int executor(char **args, char **env);

// Helpers
int myStrcmp(const char *a, const char *b);
char *myGetenv(const char *name, char **env);
int myStrlen(const char *str);
int myStrnicmp(const char *a, const char *b, int n);
char *myStrdup(const char *str);
void myStrcpy(char *str1, const char *str2);
char *myStrchr(const char *input, const char delimiter);
char *parseString(char *str, char **env);

// Built in function implementation
int commandCd(char **args, char *initialDirectory);
int commandPwd();
int commandEcho(char **args, char **env);
int commandEnv(char **envp);
int commandWhich(char **args, char **env);

char **commandExport(char **args, char **env);
char **commandUnset(char **args, char **env);

#endif
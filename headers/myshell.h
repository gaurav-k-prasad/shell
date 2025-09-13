#ifndef MYSHELL_H
#define MYSHELL_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <limits.h>
#include <fcntl.h>

// Structures
#include "../structs/parser.h"

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

#define MAX(A, B) (A > B) ? A : B
#define MIN(A, B) (A < B) ? A : B

// Main helpers
Token **getTokens(char *input, char **env);
void freeEnv(char **env); // free enviornment variable string
// @deprecated
int shellBuilts(char ***args, char **env, char *initialDirectory);    // handles shell builtin commands
char *getFullPathOfWhich(char *command, char **env);                  // get's full path of a command eg ls -> /urs/bin/ls
char **cloneEnv(char **env);                                          // clones the initial enviornment
Commands *splitCommands(Token **allTokens);                           // splits the given commands as input into proper format
bool isBuiltin(char *command);                                        // find if it's a builtin command
bool isMyImplementedBulitin(char *command);                           // find if it's my implemented builtin command
int handleBuiltin(char **args, char ***env, char *initialDirectory); // if builtin then handle the builtin
int handleMyImplementedBulitin(char **args, char ***env, char *initialDirectory);

// Executor
// @deprecated
int executor(char ***args, char **env); // executes the binary files
int executeCommand(Command *command, char ***env, char *initialDirectory);
int executePipeline(Pipeline *pipeline, char ***env, char *initialDirectory);
int executePipelineComponent(
    PipelineComponent *pc, char ***env, int fds[][2],
    int pipeCount, int i, int pids[], char *initialDirectory);

// Helpers
int myStrcmp(const char *a, const char *b);              // strcmp
char *myGetenv(const char *name, char **env);            // get value of enviornment variable 'name'
int myStrlen(const char *str);                           // strlen
int myStrnicmp(const char *a, const char *b, int n);     // strnicmp
char *myStrdup(const char *str);                         // strdup
void myStrcpy(char *str1, const char *str2);             // strcpy
char *myStrchr(const char *input, const char delimiter); // strchr
bool isSemicolon(Token *input);                          // Checks if the token is a semicolon
bool isLogicalOp(Token *input);                          // Checks if the token is a logical operator (&& or ||)
bool isPipe(Token *input);                               // Checks if the token is a pipe (|)
bool isLt(Token *input);                                 // Checks if the token is a less-than operator (<)
bool isGt(Token *input);                                 // Checks if the token is a greater-than operator (>)
PipelineComponent *createPipelineComponent();            // mallocs the required item
Pipeline *createPipeline();                              // mallocs the required item
Command *createCommand();                                // mallocs the required item
Commands *createCommands();                              // mallocs the required item
void closePipes(int fds[][2], int n);                    // close pipes
void findInOutFileAndCommandEnd(
    PipelineComponent *pc, int tokensLen, Token **tokens,
    char **infile, char **outfile, int *commandEnd); // finds if there are files in the given pipeline component and gives the command end in args
bool isDelimiter(Token *token);

// Built in function implementation
int commandCd(char **args, char *initialDirectory); // cd command
int commandPwd();                                   // pwd command
int commandEcho(char **args, char **env);           // echo command
int commandEnv(char **envp);                        // env command
int commandWhich(char **args, char **env);          // which command
char **commandExport(char **args, char **env);      // export command for env
char **commandUnset(char **args, char **env);       // unset command for env

#endif
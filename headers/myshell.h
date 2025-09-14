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

#define MAX(A, B) (A > B) ? A : B
#define MIN(A, B) (A < B) ? A : B

// ! Input Parsing

/**
 * @brief Extract the tokens from input
 * @warning Free required
 *
 * @param input input string
 * @param env enviornment variable array
 * @return VectorToken* Vector of tokens
 */
VectorToken *getTokens(char *input, char **env);

/**
 * @brief Splits the tokens into parsed output
 * @warning Free required
 *
 * @param tokenVec vector of tokens
 * @return Commands* All the commands nested and ready to be used
 */
Commands *splitCommands(VectorToken *tokenVec);

/**
 * @brief Create a Token
 * @warning Free required
 *
 * @param str string to copy to new token
 * @param isOperator is it an operator
 * @return Token*
 */
Token *createToken(char *str, bool isOperator);

/**
 * @brief Create a Pipeline Component object
 * @warning Free required
 *
 * @return PipelineComponent*
 */
PipelineComponent *createPipelineComponent();

/**
 * @brief Create a Pipeline object
 * @warning Free required
 *
 * @return Pipeline*
 */
Pipeline *createPipeline();

/**
 * @brief Create a Command object
 * @warning Free required
 *
 * @return Command*
 */
Command *createCommand();

/**
 * @brief Create a Commands object
 * @warning Free required
 *
 * @return Commands*
 */
Commands *createCommands();

// ! Helpers

/**
 * @brief Get the Full Path Of given command
 * @warning Free required
 *
 * @param command command whose full path needs to be found
 * @param env enviornment variables
 * @return char* path
 */
char *getFullPathOfWhich(char *command, char **env);

/**
 * @brief clones the given enviornment variables
 * @warning Free required
 *
 * @param env enviornment variables
 * @return char**
 */
char **cloneEnv(char **env);

/**
 * @brief Handles the built in commands
 *
 * @param args Arguments given via terminal
 * @param env enviornment variables
 * @param initialDirectory current working directory
 * @return int status: returns 0 if success
 */
int handleBuiltin(char **args, char ***env, char *initialDirectory);

/**
 * @brief Handles my implemented commands
 *
 * @param args Arguments given via terminal
 * @param env enviornment variables
 * @param initialDirectory current working directory
 * @return int status: returns 0 if success
 */
int handleMyImplementedBuiltin(char **args, char ***env, char *initialDirectory);

// ! Executors

/**
 * @brief executes the given Command
 *
 * @param command the Command to execute
 * @param env reference to enviornment variables
 * @param initialDirectory current working directory
 * @return int status: returns 0 if success
 */
int executeCommand(Command *command, char ***env, char *initialDirectory);

/**
 * @brief executes the pipeline
 *
 * @param pipeline the Pipeline to execute
 * @param env reference to enviornment variables
 * @param initialDirectory current working directory
 * @return int status: returns 0 if success
 */
int executePipeline(Pipeline *pipeline, char ***env, char *initialDirectory);

/**
 * @brief Executes the Pipeline Component
 *
 * @param pc Pipeline component
 * @param env reference to enviornment variables
 * @param fds all the file descriptors from piping of pipeline
 * @param pipeCount length of file descriptor array
 * @param i current file descriptor that the pipeline component needs to attach to
 * @param pids reference of array where the child process pid will be stored
 * @param initialDirectory current working directory
 * @return int status: returns 0 if success
 */
int executePipelineComponent(
    PipelineComponent *pc, char ***env, int fds[][2],
    int pipeCount, int i, int pids[], char *initialDirectory);

// ! Basic helpers

/**
 * @brief strcmp
 *
 * @param a string 1
 * @param b string 2
 * @return int 0 if equal, -1 if a < b, 1 if a > b
 */
int myStrcmp(const char *a, const char *b);

/**
 * @brief finds the enviornment variable with a given name in envs
 * @warning Free required
 *
 * @param name the name of key
 * @param env enviornemnt variables
 * @return char* value: value of given key
 */
char *myGetenv(const char *name, char **env);

/**
 * @brief strlen
 *
 * @param str given string
 * @return int len: length of string
 */
int myStrlen(const char *str);

/**
 * @brief case insensitive comparision of 2 strings till n characters
 *
 * @param a string 1
 * @param b string 2
 * @param n length till comparision required
 * @return int 0 if equal, -1 if a < b, 1 if a > b
 */
int myStrnicmp(const char *a, const char *b, int n);

/**
 * @brief duplicates the given string
 * @warning Free required
 *
 * @param str Given string
 * @return char* string: pointer to new string
 */
char *myStrdup(const char *str);

/**
 * @brief copies the content of str2 into str1 with null termination
 * @warning Give enough length for str1 as str2 and \0
 *
 * @param str1 string 1
 * @param str2 string 2
 */
void myStrcpy(char *str1, const char *str2);

/**
 * @brief gives the pointer to the delimiter in the string
 *
 * @param input string
 * @param delimiter character
 * @return char* pointer: pointer to the delimiter
 */
char *myStrchr(const char *input, const char delimiter);

// ! General Questions about a given token/command

/**
 * @brief Check if the given command is a shell builtin.
 *
 * The built-in commands include:
 *   1. cd
 *   2. export
 *   3. unset
 *   4. quit
 *   5. exit
 *
 * @param command The command string to check.
 * @return true if the command is a builtin, false otherwise.
 */
bool isBuiltin(char *command);

/**
 * @brief Check if the given command is my implemented version
 *
 * My implmemented commands are:
 *    1. echo
 *    2. pwd
 *    3. env
 *    4. which
 *
 * @param command
 * @return true
 * @return false
 */
bool isMyImplementedBuiltin(char *command);

/**
 * @brief checks if given token is a semicolon
 *
 * @param input
 * @return true
 * @return false
 */
bool isSemicolon(Token *input);

/**
 * @brief checks if given token is a && or ||
 *
 * @param input
 * @return true
 * @return false
 */
bool isLogicalOp(Token *input);

/**
 * @brief checks if given token is a pipe |
 *
 * @param input
 * @return true
 * @return false
 */
bool isPipe(Token *input);

/**
 * @brief checks if given token is a less than operator <
 *
 * @param input
 * @return true
 * @return false
 */
bool isLt(Token *input);

/**
 * @brief checks if given token is a greater than operator >
 *
 * @param input
 * @return true
 * @return false
 */
bool isGt(Token *input);

/**
 * @brief closes all the pipes in the filedescriptor array
 *
 * @param fds file descriptors
 * @param n length
 */
void closePipes(int fds[][2], int n);

/**
 * @brief Given a Pipeline component finds if there is any redirectioin operator in the command
 *
 * If there's a redirection operator then what is the input file or output file
 *
 * Which index does the actual command end at
 *
 * @param pc Pipeline component
 * @param infile reference to input file
 * @param outfile reference to output file
 * @param commandEnd reference to integer of commandend
 */
void findInOutFileAndCommandEnd(
    PipelineComponent *pc, char **infile, char **outfile, int *commandEnd);

/**
 * @brief is the given token a delimiter: like ; or && or || or < or >
 *
 * @param token given token
 * @return true
 * @return false
 */
bool isDelimiter(Token *token);

// ! Built in commands
/**
 * @brief Changes the directory
 *
 * @param args the arguments to the directory
 * @param initialDirectory current working directory
 * @return int status: 0 if succeded
 */
int commandCd(char **args, char *initialDirectory);

/**
 * @brief adds the given value to enviornment variable
 * @warning Free required
 *
 * @param args arguments given
 * @param env enviornment variables
 * @return char** env: new enviornment variable
 */
char **commandExport(char **args, char **env);

/**
 * @brief removes given key from enviornment variable
 * @warning Free required
 *
 * @param args arguments given
 * @param env enviornment variables
 * @return char** env: new enviornment variable
 */
char **commandUnset(char **args, char **env);

// ! My implemented commands
/**
 * @brief prints current working directory
 *
 * @return int status: if success returns 0
 */
int commandPwd();

/**
 * @brief echos the given arguments
 *
 * @param args the arguments from terminal
 * @param env enviornment variables
 * @return int status: if success returns 0
 */
int commandEcho(char **args, char **env);

/**
 * @brief prints the enviornment variables
 *
 * @param envp enviornment variables
 * @return int success: if success returns 0
 */
int commandEnv(char **envp);

/**
 * @brief Finds the directory of a given command
 *
 * @param args the arguments from terminal
 * @param env enviornment variables needed to find path
 * @return int status: if success returns 0
 */
int commandWhich(char **args, char **env);

// ! Free the memory

/**
 * @brief Free previous enviornment variable
 *
 * @param env array of enviornment variables
 */
void freeEnv(char **env);

/**
 * @brief Free the given token
 *
 * @param token
 */
void freeToken(Token *token);

/**
 * @brief Free all the contents of token vector
 *
 * @param vec
 */
void freeVecToken(VectorToken *vec);

/**
 * @brief Frees all the content of pipeline_component
 *
 * @param pc
 */
void freePipelineComponent(PipelineComponent *pc);

/**
 * @brief Free all the contents of pipeline_component vector
 *
 * @param vec
 */
void freeVecPipelineComponent(VectorPipelineComponent *vec);

/**
 * @brief Free all the contents of pipeline
 *
 * @param p
 */
void freePipeline(Pipeline *p);

/**
 * @brief Free all the contents of pipeline vector
 *
 * @param vec
 */
void freeVecPipeline(VectorPipeline *vec);

/**
 * @brief Free all the contents of command
 *
 * @param c
 */
void freeCommand(Command *c);

/**
 * @brief Free all the contents of command vector
 *
 * @param vec
 */
void freeVecCommand(VectorCommand *vec);

/**
 * @brief Free all the contents of Commands
 *
 * @param c
 */
void freeCommands(Commands *cs);
#endif
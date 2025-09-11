#ifndef DECLARE
#define DECLARE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_TOKENS 1024

#define DEFINE_VECTOR(type)                                   \
  typedef struct                                              \
  {                                                           \
    type *data;                                               \
    size_t size;                                              \
    size_t capacity;                                          \
  } Vector_##type;                                            \
                                                              \
  void init_##type(Vector_##type *v, size_t cap)              \
  {                                                           \
    v->data = malloc(cap * sizeof(type));                     \
    v->size = 0;                                              \
    v->capacity = cap;                                        \
  }                                                           \
                                                              \
  void push_##type(Vector_##type *v, type val)                \
  {                                                           \
    if (v->size == v->capacity)                               \
    {                                                         \
      v->capacity *= 2;                                       \
      v->data = realloc(v->data, v->capacity * sizeof(type)); \
    }                                                         \
    v->data[v->size++] = val;                                 \
  }                                                           \
                                                              \
  void free_##type(Vector_##type *v)                          \
  {                                                           \
    free(v->data);                                            \
    v->data = NULL;                                           \
    v->size = v->capacity = 0;                                \
  }

enum Seperator
{
  AND,
  OR
};

typedef struct Token
{
  bool isOperator;
  char *token; // token
} Token;

typedef struct PipelineComponent
{
  Token **tokens; // tokens
  int n;          // number of tokens
  bool isLt, isGt;
} PipelineComponent;

typedef struct Pipeline
{
  PipelineComponent **components; // components of pipeline
  int n;                          // number of components
} Pipeline;

typedef struct Command
{
  Pipeline **pipelines; // all pipelines
  int n;                // number of pipelines
  int separator;        // all separators eg. ||, &&
} Command;

typedef struct Commands
{
  Command **commands; // all the commands
  int n;              // number of commands
} Commands;

typedef struct EnviornmentVariableReplace
{
  char *env;
  int valLength;
  int varNameLength;
} EnviornmentVariableReplace;

Token **getTokens(char *input);

#endif
#ifndef PARSER_STRUCTURES_H
#define PARSER_STRUCTURES_H

#define MAX_TOKENS 1024

#define DEFINE_VECTOR(type)                                              \
  typedef struct                                                         \
  {                                                                      \
    type **data;                                                         \
    size_t size;                                                         \
    size_t capacity;                                                     \
  } Vector_##type;                                                       \
                                                                         \
  static inline void init_##type(Vector_##type *v, size_t cap)           \
  {                                                                      \
    v->data = (type **)malloc(cap * sizeof(type *));                     \
    v->size = 0;                                                         \
    v->capacity = cap;                                                   \
  }                                                                      \
                                                                         \
  static inline void push_##type(Vector_##type *v, type *val)            \
  {                                                                      \
    if (v->size == v->capacity)                                          \
    {                                                                    \
      v->capacity *= 2;                                                  \
      v->data = (type **)realloc(v->data, v->capacity * sizeof(type *)); \
    }                                                                    \
    v->data[v->size++] = val;                                            \
  }                                                                      \
                                                                         \
  static inline void free_##type(Vector_##type *v)                       \
  {                                                                      \
    free(v->data);                                                       \
    v->data = NULL;                                                      \
    v->size = v->capacity = 0;                                           \
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
DEFINE_VECTOR(Token)

typedef struct PipelineComponent
{
  Vector_Token *tokens; // tokens
  bool isLt, isGt;
} PipelineComponent;
DEFINE_VECTOR(PipelineComponent)

typedef struct Pipeline
{
  Vector_PipelineComponent *components; // components of pipeline
  int separator;                        // next separator eg. ||, &&
} Pipeline;
DEFINE_VECTOR(Pipeline)

typedef struct Command
{
  Vector_Pipeline *pipelines; // all pipelines
} Command;
DEFINE_VECTOR(Command)

typedef struct Commands
{
  Vector_Command *commands; // all the commands
} Commands;

typedef struct EnviornmentVariableReplace
{
  char *env;
  int valLength;
  int varNameLength;
} EnviornmentVariableReplace;

#endif
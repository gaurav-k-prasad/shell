#ifndef PARSER_STRUCTURES_H
#define PARSER_STRUCTURES_H

/**
 * @file parser.h
 * @brief Data structures and macros for shell parser components.
 */

/**
 * @def MAX_TOKENS
 * @brief Maximum number of tokens supported.
 */
#define MAX_TOKENS 1024

/**
 * @brief Macro to define a resizable vector for any pointer type.
 *
 * This macro generates a struct and functions for a dynamic array of pointers to the given type.
 *
 * @param type The type to store pointers to in the vector.
 */
#define DEFINE_VECTOR(type)                                                    \
  /**                                                                          \
   * @struct Vector##type                                                      \
   * @brief Dynamic array of pointers to @a type.                              \
   * @var Vector##type::data Pointer to array of pointers to @a type.          \
   * @var Vector##type::size Number of elements in the vector.                 \
   * @var Vector##type::capacity Allocated capacity of the vector.             \
   */                                                                          \
  typedef struct                                                               \
  {                                                                            \
    type **data;                                                               \
    size_t size;                                                               \
    size_t capacity;                                                           \
  } Vector##type;                                                              \
                                                                               \
  /**                                                                          \
   * @brief Initialize a vector with a given capacity.                         \
   * @param v Pointer to the vector.                                           \
   * @param cap Initial capacity.                                              \
   */                                                                          \
  static inline void initVec##type(Vector##type *v, size_t cap)                \
  {                                                                            \
    v->data = (type **)malloc((cap + 1) * sizeof(type *));                     \
    v->size = 0;                                                               \
    v->capacity = cap;                                                         \
    v->data[0] = NULL;                                                         \
  }                                                                            \
                                                                               \
  /**                                                                          \
   * @brief Add an element to the vector, resizing if needed.                  \
   * @param v Pointer to the vector.                                           \
   * @param val Pointer to the value to add.                                   \
   */                                                                          \
  static inline void pushVec##type(Vector##type *v, type *val)                 \
  {                                                                            \
    if (v->size == v->capacity)                                                \
    {                                                                          \
      v->capacity *= 2;                                                        \
      v->data = (type **)realloc(v->data, (v->capacity + 1) * sizeof(type *)); \
    }                                                                          \
    v->data[v->size++] = val;                                                  \
    v->data[v->size] = NULL;                                                   \
  }

enum Seperator
{
  AND, /**< Logical AND (&&) */
  OR   /**< Logical OR (||) */
};

typedef struct Token
{
  bool isOperator; /**< True if the token is an operator. */
  char *token;     /**< The string value of the token. */
} Token;
DEFINE_VECTOR(Token)

typedef struct PipelineComponent
{
  VectorToken *tokens; /**< Vector of tokens in this component. */
  bool isLt;           /**< True if component contains '<'. */
  bool isGt;           /**< True if component contains '>'. */
} PipelineComponent;
DEFINE_VECTOR(PipelineComponent)

typedef struct Pipeline
{
  VectorPipelineComponent *components; /**< Vector of pipeline components. */
  int separator;                       /**< Next separator (||, &&). */
} Pipeline;
DEFINE_VECTOR(Pipeline)

typedef struct Command
{
  VectorPipeline *pipelines; /**< Vector of all pipelines. */
} Command;
DEFINE_VECTOR(Command)

typedef struct Commands
{
  VectorCommand *commands; /**< Vector of all commands. */
} Commands;

typedef struct EnviornmentVariableReplace
{
  char *env;         /**< Environment variable string. */
  int valLength;     /**< Length of the value. */
  int varNameLength; /**< Length of the variable name. */
} EnviornmentVariableReplace;

#endif

#ifndef EXECUTORS_H
#define EXECUTORS_H
#include "parser.h"

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

#endif
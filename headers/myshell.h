#ifndef MYSHELL_H
#define MYSHELL_H

#define _POSIX_C_SOURCE 200809L
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
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

#define MAX(A, B) (A > B) ? A : B
#define MIN(A, B) (A < B) ? A : B
#define RIGHT "\033[C"
#define LEFT "\033[D"
#define UP "\033[1A"
#define DOWN "\033[1B"
#define CLEAR_LINE "\033[2K\r"

#define PROMPT "$ "
#define AI_PROMPT "> "

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

#define MAX_AI_COMMANDS 256
#define MAX_AI_QUESTIONS 256

#include "../structs/parser.h"
#include "../structs/ai.h"
#include "basicHelpers.h"
#include "builtins.h"
#include "executors.h"
#include "helpers.h"
#include "input.h"
#include "parser.h"
#include "forgettingDLL.h"
#include "aiParser.h"

#endif
#ifndef INPUT_H
#define INPUT_H
#include "../structs/forgettingDLL.h"

/**
 * @brief Enables the raw mode for the terminal
 *
 */
void enableRawMode();

/**
 * @brief Disables raw mode on terminal
 *
 */
void disableRawMode();

/**
 * @brief Get the Input String from the terminal
 *
 * @param ForgettingDoublyLinkedList reference history
 * @param input reference to where to store the output
 * 
 * @note if any text written on terminal and window size change it would run whatever is written
 * 
 * @warning Free required
 * @return int status -1 if failed 1 if window size changed and no input written 0 if no error
 */
int getInputString(ForgettingDoublyLinkedList *history, char **input);

/**
 * @brief Get input from non-interactive source (pipe or file)
 *
 * @param input reference to where to store the output
 * 
 * @warning Free required
 * @return int status -1 if failed, 1 if EOF, 0 if success
 */
int getNonInteractiveInput(char **input);

#endif
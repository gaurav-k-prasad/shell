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
 * @brief clears text till len length
 *
 * @param len how many characters to clear
 */
void clearText(int len);

/**
 * @brief Get the Input String from the terminal
 *
 * @param ForgettingDoublyLinkedList reference history
 *
 * @warning Free required
 * @return char* Input String
 */
char *getInputString(ForgettingDoublyLinkedList *history);

#endif
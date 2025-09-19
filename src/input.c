#include "../headers/myshell.h"

extern struct termios orig_termios;

void enableRawMode()
{
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);
  raw.c_oflag |= OPOST; // enable post-processing
  raw.c_oflag |= ONLCR; // map NL to CR-NL

  // Set read timeout so it doesn't block forever
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  // Apply the new attributes
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    exit(EXIT_FAILURE);
}

void disableRawMode()
{
  // Restore the original terminal attributes
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
  {
    exit(EXIT_FAILURE);
  }
}

void clearText(int len)
{
  for (int i = 0; i < len; i++)
  {
    write(STDOUT_FILENO, "\b \b", 3);
  }
}

char *getInputString(ForgettingDoublyLinkedList *history)
{
  char *buffer = NULL;
  size_t buffer_size = 10; // initial buffer length
  size_t lastPosition = 0; // last position of string
  size_t currPosition = 0; // incase the caret goes left by <- key
  char c;

  buffer = malloc(buffer_size);
  if (buffer == NULL)
  {
    return NULL;
  }
  ListNode *currentHistory = NULL; // current History

  while (1)
  {
    // Use read() to get one byte from standard input
    ssize_t nread = read(STDIN_FILENO, &c, 1);
    if (nread == -1)
    {
      if (errno == EINTR) // EINTR means the function was interrupted by a signal!
      {
        clearerr(stdin);
        return NULL;
      }
      else
      {
        perror("getline");
        exit(EXIT_FAILURE);
      }
    }
    else if (nread == 0) // means no reading done
    {
      continue;
    }
    else if (c == '\n') // return the string
    {
      write(STDOUT_FILENO, "\n", 1);
      break;
    }
    else if (c == EOF || c == 4) // 4 is EOT(ASCII) for Ctrl+D
    {
      write(STDOUT_FILENO, "\n", 1);
      break;
    }
    else if (c == 12 || c == 13 || c == 11) //  Ctrl + L   Ctrl + M   Ctrl + K
    {
      continue;
    }
    else if (c == '\x1b') // '\x1b' is the escape character
    {
      // Check for arrow keys (Escape Sequences)
      char seq[5] = {};
      // Try to read the rest of the escape sequence
      read(STDIN_FILENO, &seq[0], 5);

      if (seq[0] == '[') // for ansi escape sequence
      {
        if (history->size > 0 && seq[1] == 'A') // Up key or Previous history
        {
          // clear previous text on terminal
          clearText(lastPosition);

          if (!currentHistory)
          {
            currentHistory = history->tail;
          }
          else
          {
            currentHistory = getPrevNode(currentHistory);
          }

          int len = myStrlen(currentHistory->command);

          // update the lengths to history command length
          lastPosition = len;
          currPosition = len;

          // update the buffer with new history string
          myStrcpy(buffer, currentHistory->command);
          write(STDOUT_FILENO, buffer, len);
        }
        else if (history->size > 0 && seq[1] == 'B') // Down key or next history
        {
          // clear previous text on terminal
          clearText(lastPosition);

          currentHistory = getNextNode(currentHistory);
          if (!currentHistory)
          {
            lastPosition = 0;
            currPosition = 0;
            buffer[0] = '\0';
          }
          else
          {
            int len = myStrlen(currentHistory->command);

            // update the lengths to history command length
            lastPosition = len;
            currPosition = len;

            // update the buffer with new history string
            myStrcpy(buffer, currentHistory->command);
            write(STDOUT_FILENO, buffer, len);
          }
        }
        else if (currPosition < lastPosition && seq[1] == 'C') // Left key
        {
          // go right
          write(STDOUT_FILENO, RIGHT, 3);
          currPosition++;
        }
        else if (currPosition > 0 && seq[1] == 'D') // Right key
        {
          // go left
          write(STDOUT_FILENO, LEFT, 3);
          currPosition--;
        }
        else if (seq[1] == '1' && seq[2] == ';' && seq[3] == '5') // if control + arrow keys
        {
          if (seq[4] == 'D') // Arrow Left
          {
            int leftPos = currPosition;
            // go past spaces
            while (leftPos > 0 && buffer[leftPos - 1] == ' ')
            {
              printf(LEFT);
              leftPos--;
            }
            // go past letters
            while (leftPos > 0 && buffer[leftPos - 1] != ' ')
            {
              printf(LEFT);
              leftPos--;
            }
            fflush(stdout);
            currPosition = leftPos;
          }
          else if (seq[4] == 'C') // Arrow right
          {
            int rightPos = currPosition;
            // go past spaces
            while (rightPos < lastPosition && buffer[rightPos] == ' ')
            {
              printf(RIGHT);
              rightPos++;
            }
            // go past letters
            while (rightPos < lastPosition && buffer[rightPos] != ' ')
            {
              printf(RIGHT);
              rightPos++;
            }
            fflush(stdout);
            currPosition = rightPos;
          }
        }
      }
    }
    else if (c == 8 || c == 127) // backspace
    {
      if (currPosition > 0)
      {
        for (int i = currPosition; i < lastPosition; i++)
        {
          buffer[i - 1] = buffer[i];
          // going right coz we have to reach the end of the string to delete all the characters at once
          printf(RIGHT);
        }
        fflush(stdout);
        // clear the command
        clearText(lastPosition);
        lastPosition--;
        currPosition--;
        // write the buffer command
        write(STDOUT_FILENO, buffer, lastPosition);
        // reposition the cursor to where it was before
        for (int i = lastPosition; i > currPosition; i--)
          printf(LEFT);
        fflush(stdout);
      }
    }
    else if (c == 23)
    {
      // for Ctrl + backspace
      // remove spaces initially
      int leftPos = currPosition;
      while (leftPos > 0 && buffer[leftPos - 1] == ' ')
      {
        leftPos--;
      }
      // on reaching a character remove it till space is reached
      while (leftPos > 0 && buffer[leftPos - 1] != ' ')
      {
        leftPos--;
      }
      int removedCount = currPosition - leftPos;
      for (int i = currPosition; i < lastPosition; i++)
      {
        buffer[leftPos++] = buffer[i];
        // going right coz we have to reach the end of the string to delete all the characters at once
        printf(RIGHT);
      }
      fflush(stdout);
      // clear the command
      clearText(lastPosition);
      lastPosition -= removedCount;
      currPosition -= removedCount;
      // write the buffer command
      write(STDOUT_FILENO, buffer, lastPosition);
      // reposition the cursor to where it was before
      for (int i = lastPosition; i > currPosition; i--)
        printf(LEFT);
      fflush(stdout);
    }
    else
    {
      // go copy all the words from current position one word later -> i->i+1 ...
      for (int i = lastPosition; i > currPosition; i--)
      {
        buffer[i] = buffer[i - 1];
        printf(RIGHT);
      }
      fflush(stdout);

      // add new character
      buffer[currPosition] = c;

      // clear current command
      clearText(lastPosition);
      lastPosition++;
      currPosition++;
      // Write new command
      write(STDOUT_FILENO, buffer, lastPosition);

      // reposition the cursor
      for (int i = lastPosition; i > currPosition; i--)
        printf(LEFT);
      fflush(stdout);
    }

    // Resize buffer if it's full
    if (lastPosition >= buffer_size)
    {
      buffer_size *= 2;
      char *temp = realloc(buffer, buffer_size);
      if (temp == NULL)
      {
        free(buffer);
        return NULL;
      }
      buffer = temp;
    }
  }

  buffer[lastPosition] = '\0';
  return buffer;
}

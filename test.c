#include <stdio.h>
#include <string.h>

char *myStrchr(const char *str, const char delimiter) {
  while (*str) {
    if (*str == delimiter) {
      return (char *)str;
    }
    str++;
  }

  return NULL;
}

int main(int argc, char const *argv[])
{
  // printf("%d", strlen("1\0"));
  printf("%s", myStrchr("bruh", 'k'));
  return 0;
}

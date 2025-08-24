#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
  // printf("%d", strlen("1\0"));
  printf("%d", strncmp("abc", "abc", 55));
  return 0;
}

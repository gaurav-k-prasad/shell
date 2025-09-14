#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>

int main()
{
  goto end;
  int a;
end:
  printf("%d", a);
  return 0;
}
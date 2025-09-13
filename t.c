#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main()
{
  int fds[2];

  pipe(fds);
  int pid = fork();

  if (pid == 0)
  {
    close(fds[1]);
    dup2(fds[0], STDIN_FILENO);
    close(fds[0]);
    char input[100];
    printf("Taking input\n");
    fgets(input, 100, stdin);

    printf("Child Writing: %s\n", input);
    _exit(0);
  }

  close(fds[0]);
  dup2(fds[1], STDOUT_FILENO);
  close(fds[1]);

  printf("bruh what are you talking about\n");
  fflush(stdout);

  wait(NULL);

  return 0;
}
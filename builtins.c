#include "myshell.h"

/*
 * `cd [path]`
 * `cd ..`
 * `cd .`
 $ TODO:
 * `cd ~`
 * `cd -`
*/
int commandCd(char **args, char *initialDirectory)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "expected an argument: cd [dir]\n");
  }
  else if (chdir(args[1]) != 0)
  {
    perror("cd");
  }

  return 0;
}

/*
 * `pwd`
 */
int commandPwd()
{
  char *cwd = getcwd(NULL, 0);
  if (cwd != NULL)
  {
    fprintf(stdout, "%s\n", cwd);
    free(cwd);
  }
  else
  {
    perror("cwd");
  }
  return 0;
}

/*
 * echo command
 * `echo $PATH $ENV`
 */
int commandEcho(char **args, char **env)
{
  int printNewLine = true;
  int start = 1;
  if (args[1] && myStrcmp(args[1], "-n") == 0)
  {
    start = 2;
    printNewLine = false;
  }

  for (int i = start; args[i] != NULL; i++)
  {
    if (args[i][0] == '$')
    {
      char *envVal = myGetenv(args[i] + 1, env);
      fprintf(stdout, "%s ", envVal);
      free(envVal);
    }
    else
    {
      fprintf(stdout, "%s ", args[i]);
    }
  }

  if (printNewLine)
    fprintf(stdout, "\n");

  return 0;
}

/*
 * prints enviornment variables
 * `env`
 */
int commandEnv(char **env)
{
  fprintf(stdout, "%-40s | %s\n", "Enviornment Variable", "Value");
  fprintf(stdout, "-------------------------------------------------------------------------------------------\n");
  for (int i = 0; env[i]; i++)
  {
    int count = 0;
    for (int j = 0; env[i][j]; j++)
    {
      if (env[i][j] == '=')
        break;
      count++;
    }

    char *buff = (char *)malloc(sizeof(char) * (count + 1));
    strncpy(buff, env[i], count);
    buff[count] = '\0';

    fprintf(stdout, "%-40s | %s\n", buff, env[i] + count + 1);
    free(buff);
  }

  return 0;
}

int commandWhich(char **args, char **env)
{
  if (args[1] == NULL) {
    fprintf(stderr, "which: expected a filename: which [file]");
    return 1;
  }

  const char *builtIns[] = {"cd", "pwd", "echo", "env", "unsetenv", "setenv", "exit", "which"};
  
  for (int i = 0; i < sizeof(builtIns) / sizeof(char *); i++) {
    if (myStrcmp(builtIns[i], args[1]) == 0) {
      fprintf(stdout, "%s: shell built-in command\n", builtIns[i]);
      return 0;
    }
  }

  char *fullpath = getFullPathOfWhich(args[1], env);

  return 0;
}
#include "myshell.h"

int myStrcmp(const char *a, const char *b)
{

  while (*a && (*a == *b))
  {
    a++;
    b++;
  }

  return *(unsigned char *)a - *(unsigned char *)b;
}

int myStrnicmp(const char *a, const char *b, int n)
{
  int i = 0;
  while (i < n && *a && (tolower(*a) == tolower(*b)))
  {
    a++;
    b++;
    i++;
  }

  if (i == n)
  {
    return 0;
  }
  return *(unsigned char *)a - *(unsigned char *)b;
}

int myStrlen(const char *str)
{
  int len = 0;

  while (*str)
  {
    len++;
    str++;
  }

  return len;
}

char *myGetenv(const char *name, char **env)
{
  if (name == NULL || env == NULL)
  {
    return NULL;
  }

  size_t nameLen = myStrlen(name);

  for (int i = 0; env[i]; i++)
  {
    if (myStrnicmp(env[i], name, nameLen) == 0 && env[i][nameLen] == '=')
    {
      int envValLen = myStrlen(env[i] + nameLen + 1);
      char *buff = (char *)malloc(sizeof(char) * (envValLen + 1));
      strcpy(buff, env[i] + nameLen + 1);
      return buff;
    }
  }
  return NULL;
}

char *getFullPathOfWhich(char *command, char **env)
{
  // Locate the path
  char *pathEnv = myGetenv("PATH", env);
  if (!pathEnv)
  {
    return NULL;
  }

  char *tok = strtok(pathEnv, ";");
  char buff[1024] = {'\0'};

  while (tok)
  {
    snprintf(buff, sizeof(buff), "%s\\%s", tok, command);
    if (access(buff, X_OK) == 0)
    {
      printf("Found: %s\n", buff);
      return NULL;
    }

    tok = strtok(NULL, ";");
  }

  return NULL;
}
#include "../headers/myshell.h"

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

      if (!buff)
      {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
      }

      strcpy(buff, env[i] + nameLen + 1);
      buff[envValLen] = '\0';
      return buff;
    }
  }
  return NULL;
}

void myStrcpy(char *str1, const char *str2)
{
  size_t strLen = myStrlen(str2);

  for (int i = 0; i < strLen; i++)
  {
    str1[i] = str2[i];
  }
  str1[strLen] = '\0';
}

char *myStrdup(const char *str)
{
  if (str == NULL)
    return NULL;
  size_t _strlen = myStrlen(str);
  char *dup = (char *)malloc(sizeof(char) * (_strlen + 1)); // caller has to free
  if (!dup) {
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
  }

  myStrcpy(dup, str);
  return dup;
}

char *myStrchr(const char *str, const char delimiter)
{
  while (*str)
  {
    if (*str == delimiter)
    {
      return (char *)str;
    }
    str++;
  }

  return NULL;
}

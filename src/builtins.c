#include "../headers/myshell.h"

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
  if (args[1] && myStrcmp(args[1], "-n") == 0) // handling -n flag for no new line character
  {
    start = 2;
    printNewLine = false;
  }

  for (int i = start; args[i]; i++) // parse all the arguments for echo
  {
    char *res = parseString(args[i], env);
    if (res == NULL)
    {
      fprintf(stderr, "string parsing failed\n");
      return 1;
    }
    fprintf(stdout, "%s ", res);
    free(res);
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
  if (env == NULL)
    return 0;

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
    if (buff == NULL)
    {
      fprintf(stderr, "malloc failed\n");
      exit(EXIT_FAILURE);
    }
    strncpy(buff, env[i], count); // get value of the enviornment variable to print
    buff[count] = '\0';

    fprintf(stdout, "%-40s | %s\n", buff, env[i] + count + 1); // formatted print key and value
    free(buff);
  }

  return 0;
}

/*
 * In your path variables finds the executable full path
 * `which [arg]`
 */
int commandWhich(char **args, char **env)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "which: expected a filename: which [file]\n");
    return 1;
  }

  // if one of these then it's a builtin command
  const char *builtIns[] = {"cd", "pwd", "echo", "env", "unsetenv", "setenv", "exit", "which"};

  for (int i = 0; i < sizeof(builtIns) / sizeof(char *); i++)
  {
    if (myStrcmp(builtIns[i], args[1]) == 0)
    {
      fprintf(stdout, "%s: shell built-in command\n", builtIns[i]);
      return 0;
    }
  }

  // look through everywhere in env PATH where the command is
  char *fullpath = getFullPathOfWhich(args[1], env);
  if (fullpath == NULL)
  {
    fprintf(stdout, "not found\n");
  }
  else
  {
    fprintf(stdout, "Found: %s\n", fullpath);
    free(fullpath);
  }

  return 0;
}

/*
 * Helper function for commandWhich
 */
char *getFullPathOfWhich(char *command, char **env)
{
  // Locate the path enviornment variable
  char *pathEnv = myGetenv("PATH", env);
  if (!pathEnv)
  {
    return NULL;
  }

  // split the PATH by separator
  char *tok = strtok(pathEnv, SEP);

  while (tok)
  {
    size_t len = myStrlen(tok) + 1 + myStrlen(command) + 1; // +1 for / and +1 for '\0'
    char *buff = (char *)malloc(sizeof(char) * len);
    if (!buff)
    {
      free(pathEnv);
      fprintf(stderr, "malloc failed\n");
      exit(EXIT_FAILURE);
    }

    if (tok[len - 1] != *PATH_SEPARATOR)
      snprintf(buff, len, "%s%s%s", tok, PATH_SEPARATOR, command);
    else
      snprintf(buff, len, "%s%s", tok, command);

    // find the binary file if it exists or not in the given path
    if (access(buff, X_OK) == 0)
    {
      free(pathEnv);
      return buff; // caller must free
    }

    free(buff);
    tok = strtok(NULL, SEP);
  }

  free(pathEnv);
  return NULL;
}

/*
 * function to set enviornment variable
 * export VAR="VALUE"
 * export PATH="$PATH:/new/path"
 */
char **commandExport(char **args, char **env)
{
  if (!args[1])
  {
    fprintf(stderr, "export: expected VAR=value\n");
    return env;
  }

  // count and allocate new enviornment variable string
  int envCount = 0;
  while (env[envCount])
    envCount++;
  char **newEnv = (char **)malloc(sizeof(char *) * (envCount + 2)); // 1 for null 1 for new env
  if (newEnv == NULL)
  {
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
  }

  // get the new enviornment variable - variable name
  char *str = args[1];
  int varnamelen = 0;

  while (*str && *str != '=')
  {
    str++;
    varnamelen++;
  }

  // making sure new enviornment variable has a value associated with it
  if (*str != '=')
  {
    free(newEnv);
    return env;
  }

  // actually copy the varname
  char *varname = (char *)malloc(sizeof(char) * (varnamelen + 1));
  if (varname == NULL)
  {
    fprintf(stderr, "malloc failed\n");
    free(newEnv);
    exit(EXIT_FAILURE);
  }
  strncpy(varname, args[1], varnamelen);
  varname[varnamelen] = '\0';
  str++; // skip '='

  // get the value by expanding the value string eg $PATH => /usr/bin:/usr/local/bin...
  char *varvalue = parseString(str, env);
  if (varvalue == NULL)
  {
    fprintf(stderr, "string parsing failed\n");
    free(varname);
    free(newEnv);
    return env;
  }

  int k = 0; // pointer for newEnv variable
  size_t len;
  char *curr;
  newEnv[k] = NULL;

  for (int i = 0; env[i]; i++)
  {
    // copy the others into newEnv
    if (!(myStrnicmp(env[i], varname, varnamelen) == 0 && env[i][varnamelen] == '='))
    {
      len = myStrlen(env[i]);
      curr = (char *)malloc(sizeof(char) * (len + 1));
      if (!curr) // malloc failed and delete everything
      {
        fprintf(stderr, "malloc failed\n");
        free(varname);
        free(varvalue);
        freeEnv(newEnv);
        exit(EXIT_FAILURE);
      }
      myStrcpy(curr, env[i]);
      newEnv[k++] = curr;
      newEnv[k] = NULL;
    }
  }

  // copy the new enviornment variable
  len = varnamelen + 1 + strlen(varvalue);         // 1 for '='
  curr = (char *)malloc(sizeof(char) * (len + 1)); // 1 for \0
  if (!curr)
  {
    fprintf(stderr, "malloc failed\n");
    free(varname);
    free(varvalue);
    freeEnv(env);
    exit(EXIT_FAILURE);
  }

  snprintf(curr, len + 1, "%s=%s", varname, varvalue);
  newEnv[k++] = curr;
  newEnv[k] = NULL;

  // free the memory
  free(varname);
  free(varvalue);
  freeEnv(env);

  return newEnv;
}

/**
 * @brief sets the enviornment variable
 *
 * @param args arguments passed to the command
 * @param env enviornment variables
 * @return char**
 */
char **commandUnset(char **args, char **env)
{
  if (args == NULL || args[1] == NULL || env == NULL)
  {
    fprintf(stderr, "expected argument unset [env]");
    return env;
  }

  // count env
  size_t envCount = 0;
  while (env[envCount])
    envCount++;

  if (envCount == 0)
    return env; // nothing to unset

  // in case variable not removed - makes sure there's always space for NULL
  char **newEnv = malloc(sizeof(char *) * (envCount + 1)); // removing 1 and a adding null terminator
  if (!newEnv)
  {
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
  }
  newEnv[0] = NULL;

  bool found = false;

  char *varName = args[1];
  size_t varLen = myStrlen(varName);
  int k = 0;

  for (int i = 0; env[i]; i++)
  {
    if (!found && (myStrnicmp(env[i], varName, varLen) == 0) && (env[i][varLen] == '='))
    {
      // found the enviornment variable
      found = true;
      continue;
    }

    size_t len = myStrlen(env[i]);
    newEnv[k] = (char *)malloc(sizeof(char) * (len + 1)); // +1 for '\0'
    if (!newEnv[k])
    {
      fprintf(stderr, "malloc failed\n");
      freeEnv(newEnv);
      exit(EXIT_FAILURE);
    }

    strcpy(newEnv[k], env[i]);
    k++;
    newEnv[k] = NULL;
  }

  if (!found)
  {
    freeEnv(newEnv);
    fprintf(stdout, "not found\n");
    return env; // RETURN previous enviornment variables
  }

  freeEnv(env);
  return newEnv;
}

/*
 * clone the env to modify to avoid changing original envs
 */
char **cloneEnv(char **env)
{
  if (env == NULL)
  {
    return NULL;
  }

  // count and allocate new enviornment variable string
  int envCount = 0;
  while (env[envCount])
  {
    envCount++;
  }
  char **newEnv = (char **)malloc(sizeof(char *) * (envCount + 1)); // 1 for null
  if (newEnv == NULL)
  {
    fprintf(stderr, "malloc failed\n");
    exit(EXIT_FAILURE);
  }
  int k = 0; // pointer for newEnv variable
  size_t len;
  char *curr;
  newEnv[0] = NULL;

  for (int i = 0; env[i]; i++)
  {
    // clone everything
    len = myStrlen(env[i]);
    curr = (char *)malloc(sizeof(char) * (len + 1));
    if (!curr)
    {
      freeEnv(newEnv);
      fprintf(stderr, "malloc failed\n");
      exit(EXIT_FAILURE);
    }
    myStrcpy(curr, env[i]);
    newEnv[k++] = curr;
    newEnv[k] = NULL;
  }
  return newEnv;
}

/*
 * free enviornment variables string
 */
void freeEnv(char **env)
{
  if (env == NULL)
    return;

  for (int i = 0; env[i]; i++)
  {
    free(env[i]);
  }
  free(env);
}
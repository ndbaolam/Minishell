#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/signal.h>

#define MAX_CMD_LEN 1024
#define MAX_TOKENS 64
#define DELIM " \t\r\n\a"

int checkCommand(char **arr, int size, char *target) {
    int left = 0, right = size - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;

        int cmp = strcmp(arr[mid], target);

        if (cmp == 0) {
            return 1; 
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return 0;
}

void runInForeground(char **args, int N) {
    pid_t pid = fork();
    if (pid == 0) {        
        execvp(args[0], args);
        perror("execvp"); 
        exit(EXIT_FAILURE);
    } else if (pid > 0) {  
        waitpid(pid, NULL, 0);
    } else {        
        perror("fork");
    }
}

void runInBackground(char **args, int N) {
    pid_t pid = fork();
    if (pid == 0) {      
        args[--N] = NULL; 
        execvp(args[0], args);
        perror("execvp"); 
        exit(EXIT_FAILURE);
    } else if (pid < 0) {        
        perror("fork");
    }   
}


char *get_current_folder()
{
  char *working_dir = malloc(MAX_TOKENS * sizeof(char));
  if (!working_dir)
  {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  if (getcwd(working_dir, MAX_TOKENS) != NULL)
  {
    return working_dir;
  }
  else
  {
    perror("getcwd");
    free(working_dir);
    exit(EXIT_FAILURE);
  }
}

void split_line(char *line, char ***args, int *N)
{
  int bufsize = MAX_TOKENS, pos = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  if (!tokens)
  {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  char *token = strtok(line, DELIM);
  while (token != NULL)
  {
    tokens[pos++] = token;
    if (pos >= bufsize)
    {
      bufsize += MAX_TOKENS;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens)
      {
        perror("realloc");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, DELIM);
  }
  tokens[pos] = NULL;

  *args = tokens;
  *N = pos;
}

void changeDir(char **args)
{
  if (args[1] == NULL)
  {
    chdir(getenv("HOME"));
  }
  else
  {
    if (chdir(args[1]) == -1)
    {
      printf(" %s: no such directory\n", args[1]);
    }
  }
}
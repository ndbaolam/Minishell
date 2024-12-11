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

void handle_signal(int signum) {
  if(signum == SIGCHLD) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
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

int main()
{
  signal(SIGCHLD, handle_signal);
  while (1)
  {
    char **args;
    int N;
    char line[MAX_CMD_LEN] = {0};
    char *pwd = get_current_folder();

    printf("%s> ", pwd);    
    fgets(line, sizeof(line) - 1, stdin);        
    
    split_line(line, &args, &N);    

    if (N > 0 && strcmp(args[N - 1], "&") == 0)
    {
      pid_t pid = fork();
      if (pid == 0) {
        args[--N] = NULL;
        execvp(args[0], args);        
      }        
      else if (pid > 0);        
      else
        perror("fork");
    }
    else if(strncmp(args[0], "exit", 4) == 0) {
      break;
    }
    else if (N > 0)
    {
      pid_t pid = fork();
      if (pid == 0)
        execvp(args[0], args);
      else if (pid > 0)
        waitpid(pid, NULL, 0);
      else
        perror("fork");
    }

    free(args);
  }
  return 0;
}

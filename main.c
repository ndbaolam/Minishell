#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/signal.h>
#include <fcntl.h>
#include "func.c"

void handle_signal(int signum)
{
  if (signum == SIGCHLD)
  {
    while (waitpid(-1, NULL, WNOHANG) > 0)
      ;
  }
}

void executeFileInCommand(char **args, int N)
{
  int i;
  int fd;
  char *command[N];

  for (i = 0; i < N; i++)
  {
    if (strcmp(args[i], ">") == 0)
    {
      break;
    }
    command[i] = args[i];
  }

  command[i] = NULL;

  if (i == N - 1)
  {
    fprintf(stderr, "Error: No output file specified.\n");
    return;
  }

  fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0)
  {
    perror("Error opening file");
    return;
  }

  int stdout_copy = dup(STDOUT_FILENO);
  dup2(fd, STDOUT_FILENO);  

  if (fork() == 0)
  {
    execvp(command[0], command);
    perror("Error executing command");
    exit(1);
  }
  else
  {
    wait(NULL);
  }

  dup2(stdout_copy, STDOUT_FILENO);

  close(fd);
  close(stdout_copy);
}

void executePipedCommand(char **args, int N)
{
  int pipefd[2];
  pid_t pid1, pid2;
  int i;

  for (i = 0; i < N; i++)
  {
    if (strcmp(args[i], "|") == 0)
    {
      break;
    }
  }

  if (i == N || i == 0 || i == N - 1)
  {
    fprintf(stderr, "Error: Invalid pipe command.\n");
    return;
  }

  char *cmd1[i + 1];
  char *cmd2[N - i];

  for (int j = 0; j < i; j++)
  {
    cmd1[j] = args[j];
  }
  cmd1[i] = NULL;

  for (int j = i + 1, k = 0; j < N; j++, k++)
  {
    cmd2[k] = args[j];
  }
  cmd2[N - i - 1] = NULL;

  if (pipe(pipefd) == -1)
  {
    perror("pipe");
    return;
  }

  if ((pid1 = fork()) == 0)
  {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);
    execvp(cmd1[0], cmd1);
    perror("execvp");
    exit(EXIT_FAILURE);
  }

  if ((pid2 = fork()) == 0)
  {
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
    execvp(cmd2[0], cmd2);
    perror("execvp");
    exit(EXIT_FAILURE);
  }

  close(pipefd[0]);
  close(pipefd[1]);
  waitpid(pid1, NULL, 0);
  waitpid(pid2, NULL, 0);
}

void executeFileOutCommand(char **args, int N)
{
  int i;
  int fd;

  for (i = 0; i < N; i++)
  {
    if (strcmp(args[i], "<") == 0)
    {
      break;
    }
  }

  if (i == N || i == 0 || i == N - 1)
  {
    fprintf(stderr, "Error: Invalid input redirection command.\n");
    return;
  }

  char *command[i + 1];
  for (int j = 0; j < i; j++)
  {
    command[j] = args[j];
  }
  command[i] = NULL;

  fd = open(args[i + 1], O_RDONLY);
  if (fd < 0)
  {
    perror("Error opening file");
    return;
  }

  if (fork() == 0)
  {
    dup2(fd, STDIN_FILENO);
    close(fd);
    execvp(command[0], command);
    perror("execvp");
    exit(EXIT_FAILURE);
  }
  else
  {
    close(fd);
    wait(NULL);
  }
}

void runCommand(char **args, int N)
{
  if (strcmp(args[0], "cd") == 0)
  {
    changeDir(args);
  }

  if (checkCommand(args, N, ">"))
  {
    executeFileInCommand(args, N);
  }

  if (checkCommand(args, N, "|"))
  {
    executePipedCommand(args, N);
  }

  if (checkCommand(args, N, "<"))
  {
    executeFileOutCommand(args, N);
  }

  if (N > 0 && strcmp(args[N - 1], "&") == 0)
  {
    runInBackground(args, N);
  }
  else if (N > 0)
  {
    runInForeground(args, N);
  }
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

    if (strcmp(line, "\n") == 0)
      continue;
    if (strncmp(line, "exit", 4) == 0)
      break;

    split_line(line, &args, &N);
    runCommand(args, N);
    free(args);
  }
  return 0;
}

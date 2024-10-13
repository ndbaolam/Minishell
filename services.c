#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMD_LEN 1024
#define MAX_TOKENS 64
#define DELIM " \t\r\n\a"

char *read_line();
char **split_line(char *);
char *get_current_folder();
int execute(char **);
int launch(char **);
int cd_cmd(char **);
int exit_cmd(char **);
int help_cmd(char **);
int execute_background(char **);
void signal_handler(int);

char *builtin_str[] = {
  "cd",
  "exit",
  "help"
};

int (*builtin_func[]) (char **) = {
  &cd_cmd,
  &exit_cmd,
  &help_cmd
};

char *get_current_folder() {
  char *working_dir = malloc(MAX_TOKENS * sizeof(char));

  if(getcwd(working_dir, MAX_TOKENS) != NULL) {
    return working_dir;
  } else {
    perror("getcwd()");
    exit(1);
  }

  return NULL;
}

char *read_line()
{
  char *line = malloc(MAX_CMD_LEN * sizeof(char));
  fgets(line, sizeof(line), stdin);
  line[strcspn(line, "\n")] = '\0';
  return line;
}

char **split_line(char *line)
{
  int bufsize = MAX_TOKENS, pos = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token = strtok(line, DELIM);

  while (token != NULL)
  {
    tokens[pos++] = token;
    if (pos >= bufsize)
    {
      bufsize += MAX_TOKENS;
      tokens = realloc(tokens, bufsize * sizeof(char *));
    }
    token = strtok(NULL, DELIM);
  }
  tokens[pos] = NULL;
  return tokens;
}

int launch(char **args) {
  int n = 0;

  if(args[0] == NULL) {
    return 1;
  }

  for(int i = 0; i < 3; i++) {
    if(strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }  

  while(args[n++]);
  
  if(n > 0 && (args[n - 1], "&", 1) == 0) {
    printf("Running in background mode...");
    args[n - 1] = NULL;
    return execute_background(args);
  } else 
    return execute(args);
}

int execute(char **args)
{
  pid_t wpid, pid;
  int status;

  pid = fork();
  if (pid == 0) {    
    if (execvp(args[0], args) == -1)
    {
      perror("Error executing command");
      exit(EXIT_FAILURE);
    }        
    exit(EXIT_SUCCESS);
  }
  else if (pid < 0) {
    perror("fork()");
  }
  else {    
    wait(NULL);
  }

  return 1;
}

int cd_cmd(char **args) {
  if(args[1] == NULL) {
    fprintf(stderr, "expected argument to \"cd\"\n");
  } else {
    if(chdir(args[1]) != 0) {
      perror("chdir()");      
    }
  }
  return 1;
}

int exit_cmd(char **args) {
  exit(EXIT_SUCCESS);
}

int help_cmd(char **args) {
  printf("This is a my Minishell project in Operating System course term 2024.2 at HUST\n");
  printf("This shell using Linux system calls to execute command\n");
  printf("By default the commands run in  forceground, you can run command in background mode by adding & at the end\n");
  return 1;
}

int execute_background(char **args) {
  // signal(SIGCHLD, signal_handler);

  pid_t pid = fork();
  if (pid == 0) {    
    if (execvp(args[0], args) == -1)
    {
      perror("Error executing command");
      exit(EXIT_FAILURE);
    }        
  }
  else if (pid < 0) {
    perror("fork()");
  } else {
    waitpid(-1, NULL, WNOHANG);
  }

  return 1;
}

void signal_handler(int signum) {
  while(waitpid(-1, NULL, WNOHANG) > 0);
}
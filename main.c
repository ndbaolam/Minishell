#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *read_line();
char **split_line(char *);
char *get_current_folder();
int execute(char **);
int launch(char **);
int cd_cmd(char **);
int exit_cmd(char **);
int help_cmd(char **);

int main() {  
  while (1) {
    char *line, *pwd;
    char **agrs;
    int result;

    pwd = get_current_folder();

    printf("%s > ", pwd);
    
    line = read_line();
    agrs = split_line(line);

    result = launch(agrs);    

    free(line);
    free(agrs);
    free(pwd);

    if(result == 0) {
      perror("Command error");
      exit(EXIT_FAILURE);
    }
  }
  return 0;
}
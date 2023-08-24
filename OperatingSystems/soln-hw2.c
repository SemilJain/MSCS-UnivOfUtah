#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"
#define MAX_PIPE_COUNT 10
#define MAX_HISTORY 10
#define MAX_LEN 100

char history[MAX_HISTORY][MAX_LEN];
int history_count = 0;

void add_history(char *str) {
  strcpy(history[history_count % MAX_HISTORY], str);
  history_count++;
}

void print_history() {
  int start = history_count - MAX_HISTORY;
  if (start < 0) start = 0;
  for (int i = start; i < history_count; i++)
    printf("%d %s\n", i + 1, history[i % MAX_HISTORY]);
}

int implement_cd(char **args){
  if(args[1] == NULL)
    args[1] = ".";
  if(chdir(args[1]) == -1){
        printf("Error in cd command | No such directory: %s\n", args[1]);  
      }
  return 1;
}



int sh_launch(int argc, char **args){
  // printf("arg count %d\n",argc);
  if (strcmp(args[0], "history") == 0) {
    print_history();
    return 1;
  }
  
  int i,j;
  int input_redir = 0;
  int output_redir = 0;
  int input_file = 0;
  int output_file = 0;
  int pipe_count = 0;
  int bg = 0;
  int pipe_positions[MAX_PIPE_COUNT+1];
  char **new_args = malloc((argc) * sizeof(char*));
  char str[MAX_LEN] = "";
  for (i = 0, j = 0; i < argc; i++) {
    strcat(str,args[i]);
    strcat(str, " ");
    if (args[i] == NULL) {
      new_args[i] = NULL;
      break;
    }
    if (strcmp(args[i], "<") == 0) {
      input_redir = 1;
      new_args[i] = NULL;
      if (args[i + 1] != NULL) {
        input_file = open(args[i + 1], O_RDONLY);
        if (input_file == -1) {
          printf("Failed to open input file.\n");
          return 1;
        }
      }
    } else if (strcmp(args[i], ">") == 0) {
      output_redir = 1;
      new_args[i] = NULL;
      if (args[i + 1] != NULL) {
        output_file = open(args[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (output_file == -1) {
          printf("Failed to open output file.\n");
          return 1;
        }
      }
    } else if ( strcmp(args[i], "|") == 0 ) {
      pipe_positions[pipe_count++] = i;
    } else if (strcmp(args[i], "&") == 0)
    {
      bg = 1;
    }
    


    if (!input_redir && !output_redir) {
      new_args[i] = args[i];
    }
  }
  if (bg)
      {
        new_args[argc - 1] = NULL;
      }

  add_history(str);
  if (strcmp(args[0], "cd") == 0){
      if (argc > 2)
      {
        printf("Extra arguments passed for cd\n");
        return 1;
      }
      return implement_cd(args);
  }
  pipe_positions[pipe_count] = argc;
  int pid = fork();
  if(pid > 0){
    // pid = wait(NULL);
    int s;

    while (1)
    {
      pid = waitpid(pid, &s, WNOHANG);
      if (bg)
      {
        break;
      }
        
      if (pid != 0 )
      {
        break;
      }
    }
    free(new_args);
  } else if(pid == 0){

    if (input_redir && pipe_count == 0) {
      dup2(input_file, 0);
      close(input_file);
    }

    if (output_redir && pipe_count == 0) {
      dup2(output_file, 1);
      close(output_file);
    }
    if (pipe_count == 0) {
      
      if(execvp(new_args[0], new_args) == -1){
        printf("Invalid Command or arguments: %s\n", args[0]);  
      }
    } else {
      int prev_pipe_read = 0;

      int j,k;
      int l = 0;
      for(j = 0; j<=pipe_count; j++){
        char **pipe_args = malloc(argc * sizeof(char*));
        int in_redir = 0;
        int count = pipe_positions[j] - l;
        for (int i = 0; i < count; i++)
        {
          pipe_args[i] = args[l++];
          if (j == 0 && strcmp(pipe_args[i], "<") == 0) in_redir = 1;
          
        }
        pipe_args[count] = NULL;
        l++;
        int curr_pipe[2];
        pipe(curr_pipe);
        int pid = fork();
        if (pid == 0) {
          if (in_redir)
          {
            if (args[count - 1] != NULL) {
              input_file = open(args[count - 1], O_RDONLY);
            }
            dup2(input_file, 0);
            close(input_file);
            pipe_args[1] = NULL;
          }
          
          if (j > 0){
            close(0);
            dup2(prev_pipe_read, 0);
            close(prev_pipe_read);
          }

          if (j < pipe_count){
            close(1);
            dup2(curr_pipe[1], 1);
            close(curr_pipe[1]);
          }

          if(execvp(pipe_args[0], pipe_args) == -1){
            printf("Invalid Command or arguments: %s\n", pipe_args[0]);
            exit(1);
          }

        } else if (pid > 0) {

          if (j > 0)
            close(prev_pipe_read);

          close(curr_pipe[1]);
          prev_pipe_read = curr_pipe[0];
          int status;
          waitpid(pid, &status, 0);
          if (WEXITSTATUS(status) != 0) {
            printf("Invalid Command or arguments: %s\n", pipe_args[0]);
            break;
          }
        } else {
          printf("fork error while executing\n");
        }
        
      }
      // for (k = 0; k < pipe_count + 1; k++)
      //       wait(NULL);
    }
    
  } else {
    printf("fork error while executing\n");
  }
  return 1;
}

int sh_execute(int argc, char **args)
{
  if (args[0] == NULL) {
    return 1;  // An empty command was entered.
  }
  // int flag = 0;
  int l = 0;
  for (int i = 0; i <= argc; i++)
  {
    if (args[i] == NULL || strcmp(args[i], ";") == 0)
    {
      // if (/* condition */)
      // {
      //   flag = 1;
      // }
      int c = 0;
      char **cmd_args = malloc(argc * sizeof(char*));
      while (l < i)
      {
        cmd_args[c++] = args[l++];
      }
      // cmd_args[c] = NULL;
      l++;
      sh_launch(c, cmd_args);
    }
    
  }
  return 1;
}

char **sh_split_line(char *line, int *argc)
{
  int bufsize = SH_TOK_BUFSIZE;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "sh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, SH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SH_TOK_DELIM);
  }
  tokens[position] = NULL;
  *argc = position;
  return tokens;
}


// char *sh_read_line(void)
// {
//   int bufsize = 15;
//   char *str;
//   str = (char *) malloc(bufsize * sizeof(char));
//   int ch, i = 0;  
      
//   while ( (ch =getchar()) !='\n'){
//     if(ch == EOF){
//       // fprintf(stderr, "EOF\n");
//       exit(EXIT_SUCCESS);
//     }
//     str[i]= ch;
//     ++i;
//     if(i == bufsize){
//       str = (char *) realloc(str, bufsize * sizeof(char));
//   }
       
//   }
//   str[i] = 0;
//   return str;   
// }

char *sh_read_line(void)
{
  char *line = NULL;
  size_t bufsize = 0;  // have getline allocate a buffer for us

  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin))  // EOF
    {
      fprintf(stderr, "EOF\n");
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "Value of errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }
  }
  return line;
}

void sh_loop(void)
{
  char *line;
  char **args;
  int status;
  int argc;
  do {
    printf("utsh$ ");
    line = sh_read_line();
    args = sh_split_line(line, &argc);
    status = sh_execute(argc, args);
    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  sh_loop();
  return EXIT_SUCCESS;
}

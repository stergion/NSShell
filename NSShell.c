/***************************************************************************//**
  @file         NSShell.c
  @author       Stergios Nanos
*******************************************************************************/


#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define  LINESIZE   512
#define  TOK_DELIM  " \t\r\n\a"


// Shell modes.
void interactive_mode();
void batch_mode(char const *batchfileName);


// Functions
void pprommpt();
char *getLineConsole();
char **getArgs(char *line);
int simpleExecute(char **args, int waitCmd);
int execute(char **args);
char *getLineFile(FILE *fp);


int main(int argc, char const *argv[]) {
  // Check number of Arguments to decide in what mode to run.
  if(argc == 1) {
    // Run shell in Interactive mode
    interactive_mode();
  } else if(argc == 2) {
    // Run shell in Batch mode.
    batch_mode(argv[1]);
  } else {
    // Incorrect number of Arguments
    printf("Enter the correct number of arguments!!!\n");
    printf("Enter no arguments to run shell in interactive mode\n");
    printf("OR enter the name of a batchfile ot run shell in batch mode.\n");
  }

  return 0;
}


void pprommpt() {
  printf("nanos_8701> ");
}


char *getLineConsole() {
  int bufsize = LINESIZE;
  char *line = malloc(bufsize * sizeof(char));

  if(line == NULL) {
    printf("ERROR: Allocation error in function getLineConsole.\n");
    exit(EXIT_FAILURE);
  }

  fflush(stdin);
  if (fgets(line, bufsize, stdin) == NULL) {
    printf("ERROR: Can't read more than %d characters at a time\n", bufsize);
    exit(EXIT_FAILURE);
  }

  return line;
}


char **getArgs(char *line) {
  int bufsize = LINESIZE, index = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if(tokens == NULL) {
    printf("ERROR: Can't allocate memory for tokens.\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOK_DELIM);
  while (token != NULL) {
    tokens[index] = token;
    index++;

    token = strtok(NULL, TOK_DELIM);
  }
  tokens[index] = NULL;

  return tokens;
}


int simpleExecute(char **args, int waitCmd) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      printf("Invalid argument \'%s\'.\n", args[0]);
      //printf("ERROR: execvp\n");
      exit(EXIT_FAILURE);
    } else {
      return 0;
    }
  } else if (pid < 0) {
    // Error forking
    printf("ERROR: Error forking\n");
    exit(EXIT_FAILURE);
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
      if (wpid == -1) {
          printf("ERROR: waitpid\n");
          exit(EXIT_FAILURE);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    if (waitCmd == 1) {
      if (WEXITSTATUS(status) == 0) {
        return 0;
      } else {
        printf("Arguments following \'%s\' aren't executed.\n", args[0]);
        //printf("Previous comand did NOT executed successfully. Exiting...\n");
        //exit(EXIT_FAILURE);
        return EXIT_FAILURE;
      }
    } else if (waitCmd == 0) {
        return 0;   // Don't care if previous comand executed successfully or not.
    } else {
        printf("ERROR: Invalid waitCmd value: %d\n", waitCmd);
        exit(EXIT_FAILURE);
    }
  }

  return 0;
}


int execute(char **args) {
  int i = 0, j, waitCmd = 0, status;
  char **simpleArgs = malloc(LINESIZE * sizeof(char*));

  if(simpleArgs == NULL) {
    printf("ERROR: Can't allocate memory for simpleArgs.\n");
    exit(EXIT_FAILURE);
  }

  while (args[i] != NULL) {
    // first check if user wants to quit shell.

    if (strcmp(args[i], "quit") == 0) {
      free(simpleArgs);
      return -1;
    }

    // if user doesn't want to quit,
    // if there are multiple user comands
    // split them into single comands and then execute them.
    j = 0;
    do {
      simpleArgs[j] = strdup(args[i]);

      j++;
      i++;
    } while(args[i] != NULL && strcmp(args[i], ";") != 0 && strcmp(args[i], "&&") != 0);
    simpleArgs[j] = NULL;

    if (args[i] != NULL){
      if (strcmp(args[i], ";") == 0) waitCmd = 0;    // don't wait for the previous comand to finish
      if (strcmp(args[i], "&&") == 0) waitCmd = 1;   // wait for previous comand to finish and execute only if return successfully
      i++;
    }

    status = simpleExecute(simpleArgs, waitCmd);

    // free allocated memory
    j = 0;
    while (simpleArgs[j] != NULL) {
      free(simpleArgs[j++]);
    }

    if (status != 0) break;
  }

  // Free memory
  free(simpleArgs);

  return 0;
}


void interactive_mode() {
  char *line;
  char **args;
  int status;

  do {
    pprommpt();

    line = getLineConsole();
    args = getArgs(line);
    status = execute(args);

    free(line);
    free(args);
  } while(status != -1);
}


char *getLineFile(FILE *fp) {
  int bufsize = LINESIZE;
  char *line = malloc(bufsize * sizeof(char));

  if(line == NULL) {
    printf("ERROR: Allocation error in function getLineFile.\n");
    exit(EXIT_FAILURE);
  }

  if (fgets(line, bufsize, fp) == NULL) {
    if (feof(fp)) {
      strcpy(line, "quit");
    } else {
      printf("ERROR: Can't read more than %d characters at a time\n", bufsize);
      exit(EXIT_FAILURE);
    }
  }
  fflush(fp);

  return line;
}


void batch_mode(char const *batchfileName) {
  FILE *fp;
  char *line;
  char **args;
  int status;

  pprommpt();
  printf("Executing comands in file '%s' line by line\n", batchfileName);
  fflush(stdout);

  fp = fopen(batchfileName, "r");
  if(fp == NULL) {
    printf("ERROR: Can't open batchfile %s.\n", batchfileName);
    exit(EXIT_FAILURE);
  }

  do {
    printf("\n");
    line = getLineFile(fp);
    args = getArgs(line);
    status = execute(args);

    free(line);
    free(args);
  } while(status != -1);

  fclose(fp);
}

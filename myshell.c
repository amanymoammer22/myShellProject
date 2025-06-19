// myshell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "myshell.h"

#define MAX_LINE 1024

void run_shell(FILE *input);

int main(int argc, char *argv[]) {
    char *shell_path = realpath(argv[0], NULL);
    setenv("shell", shell_path, 1);

    if (argc == 2) {
        FILE *batch = fopen(argv[1], "r");
        if (!batch) {
            perror("Failed to open batch file");
            return 1;
        }
        run_shell(batch);
        fclose(batch);
    } else {
        run_shell(stdin);
    }

    free(shell_path);
    return 0;
}

void run_shell(FILE *input) {
printf("\n\n\t ************* OUR SHELL *************");
printf("\n\n\t************* Amany ,Doaa ,Mona ************* \n");
printf("\n");
    char line[MAX_LINE];

    while (1) {
        if (input == stdin) {
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            printf("%s> ", cwd);
        }

        if (!fgets(line, sizeof(line), input)) break;
        if (line[0] == '\n') continue;

        handle_command(line);
    }
}

// utility.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "myshell.h"

void execute_external(char **args, int background, char *infile, char *outfile, int append);
void redirect_output(FILE *stream, const char *outfile, int append);

void handle_command(char *line) {
    char *args[128], *infile = NULL, *outfile = NULL;
    int argc = 0, background = 0, append = 0;

    // Parse tokens
    char *token = strtok(line, " \t\n");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t\n");
            infile = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\n");
            outfile = token;
            append = 0;
        } else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " \t\n");
            outfile = token;
            append = 1;
        } else if (strcmp(token, "&") == 0) {
            background = 1;
        } else {
            args[argc++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL;
    if (argc == 0) return;

    // Internal commands
    if (strcmp(args[0], "cd") == 0) {
        if (argc == 1) {
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
        } else if (chdir(args[1]) != 0) perror("cd failed");
    }
    else if (strcmp(args[0], "clr") == 0) {
        printf("\033[H\033[J");
    }
    else if (strcmp(args[0], "dir") == 0) {
        const char *dir = (argc > 1) ? args[1] : ".";
        DIR *dp = opendir(dir);
        struct dirent *entry;
        if (!dp) { perror("dir failed"); return; }
        FILE *out = stdout;
        if (outfile) out = fopen(outfile, append ? "a" : "w");
        while ((entry = readdir(dp)) != NULL) {
            fprintf(out, "%s\n", entry->d_name);
        }
        if (outfile) fclose(out);
        closedir(dp);
    }
    else if (strcmp(args[0], "environ") == 0) {
        extern char **environ;
        FILE *out = stdout;
        if (outfile) out = fopen(outfile, append ? "a" : "w");
        for (char **env = environ; *env; env++) {
            fprintf(out, "%s\n", *env);
        }
        if (outfile) fclose(out);
    }
    else if (strcmp(args[0], "echo") == 0) {
        FILE *out = stdout;
        if (outfile) out = fopen(outfile, append ? "a" : "w");
        for (int i = 1; i < argc; i++) {
            fprintf(out, "%s ", args[i]);
        }
        fprintf(out, "\n");
        if (outfile) fclose(out);
    }
    else if (strcmp(args[0], "pause") == 0) {
        printf("Press Enter to continue...");
        getchar();
    }
    else if (strcmp(args[0], "help") == 0) {
        char path[1024];
        snprintf(path, sizeof(path), "more readme");
        system(path); 
    }
    else if (strcmp(args[0], "quit") == 0) {
        exit(0);
    }
    else {
        // External command
        execute_external(args, background, infile, outfile, append);
    }
}

void execute_external(char **args, int background, char *infile, char *outfile, int append) {
    pid_t pid = fork();
    if (pid == 0) {
        setenv("parent", getenv("shell"), 1);
        if (infile) {
            int fd = open(infile, O_RDONLY);
            if (fd < 0) { perror("input redirect"); exit(1); }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (outfile) {
            int fd = open(outfile, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
            if (fd < 0) { perror("output redirect"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    } else {
        if (!background) waitpid(pid, NULL, 0);
    }
}

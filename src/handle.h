#ifndef HANDLE_H
#define HANDLE_H

#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
//#include "builtin.h"
#include "profile.h"

int normal_handle(char ** args, int fd, int start);
int redirect_handle(char **args, int fd_in, int fd_out, int start);

int pipe_handle(char **args, int fd_in, int start);

int lsh_handle(char **args, int fd_in, int fd_out,int start, int type) {
    switch (type)
    {
    case NORMAL:
        return normal_handle(args, fd_in, start);
        break;
    case REDIRECT_INPUT:
        return redirect_handle(args, fd_in, fd_out, start);
        break;
    case REDIRECT_OUTPUT:
        return redirect_handle(args,fd_in, fd_out, start);
        break;
    case PIPE:
        return pipe_handle(args, fd_in, start);
        break;
    default:
        break;
    }
}

int normal_handle(char ** args, int fd, int start) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if( pid == 0) {
        // child process
        //printf("launch : %s\n", args[0]);
        if(fd != -1) {
            dup2(fd, STDIN_FILENO);
        }
        if(execvp(args[start], args+start) == -1) {
            perror("exec error");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        // Error forking
        perror("fork error");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int redirect_handle(char **args, int fd_in, int fd_out, int start) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if( pid == 0) {
        // child process
        if(fd_in != -1) {
            dup2(fd_in, STDIN_FILENO);
        }
        if(fd_out != -1) {
            dup2(fd_out, STDOUT_FILENO);
        }
        if(execvp(args[start], args+start) == -1) {
            perror("exec error");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        // Error forking
        perror("fork error");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    if(fd_in != -1) {
        close(fd_in);
    }
    if(fd_out != -1) {
        close(fd_out);
    }

    return 1;
}

int pipe_handle(char **args, int fd_in, int start){
    int fd[2];
    int status;
    pid_t pid, wpid;
    // create pipe
    int ret = pipe(fd);
    if(ret == -1) {
        perror("pipe error");
    }

    pid = fork();
    if(pid == 0) {
        // use created pipe to redirect
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        if(fd_in != -1) {
            dup2(fd_in, STDIN_FILENO);
        }
        execvp(args[start], args+start);
    } else if(pid < 0) {
        perror("fork error");
    } else {
        // parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    if(fd_in != -1) {
        close(fd_in);
    }
    close(fd[1]);
    return fd[0];
}

#endif
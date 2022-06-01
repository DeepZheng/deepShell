#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include "function.h"



int lsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if( pid == 0) {
        // child process
        //printf("launch : %s\n", args[0]);
        if(execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


char *lsh_read_line() {
    char *line = NULL;
    ssize_t bufsize = 0;

    if(getline(&line, &bufsize, stdin) == -1){
        if(feof(stdin)){
            exit(EXIT_SUCCESS);
        } else {
            perror("readline error");
            exit(EXIT_FAILURE);
        }
    } 
    //printf("readline : %s\n", line);
    return line;
} 

char *lsh_get_line(char *name, char *path) {
    char *buf = malloc(sizeof(char) * LSH_LINE_BUFSIZE);
    memset(buf, 0, sizeof(char) * LSH_LINE_BUFSIZE);
    struct termios old_opt, opt;
    char ch;
    int maxposition = history_count;
    int position = maxposition;
      
    //printf("this is test!\n");

    tcgetattr(0, &old_opt);
    opt = old_opt;
    
    opt.c_lflag &= ~ECHO;
    opt.c_lflag &= ~ICANON;
    
    tcsetattr(0, TCSANOW, &opt); // new setup

    
    printf("[%s@%s] >> ",name, path);
    while(1)
    {
        ch = getchar();
        if( ch == 27 ) {
            getchar();
            if( (ch = getchar()) == 65 || ch == 66 ) {
                if(ch == 65) {
                    // key up
                    int i = strlen(buf);
                    //memset(buf, 0, 100);
                    if(position > 0) {
                        strcpy(buf, history_command[--position]);
                    } else {
                        strcpy(buf, "");
                    }
                    printf("\r[%s@%s] >> ",name, path);
                    for(int j = 0; j < i; ++j)
                    {
                        printf(" ");

                    }
                    printf("\r[%s@%s] >> %s",name, path, buf);
                    fflush(stdout);
                }
                else if(ch == 66) {   
                    // key down
                    int i = strlen(buf);
                    if(position < maxposition){
                        strcpy(buf, history_command[++position]);
                    } else {
                        strcpy(buf, "");
                    }
                    printf("\r[%s@%s] >> ",name, path);
                    for(int j = 0; j < i; ++j)
                    {
                        printf(" ");

                    }
                    printf("\r[%s@%s] >> %s",name, path, buf);
                    fflush(stdout);
                }
            }
        }
        else if(ch == 127) {
            // delete
            int i = strlen(buf);
            if(i != 0)
            {
                buf[i-1] = '\0';
                i--;
                printf("\r[%s@%s] >> ",name, path);
                for(int j = 0;j < i + 1; ++j){
                    printf(" ");//覆盖掉自己

                }
                
                fflush(stdout);
                
                printf("\r[%s@%s] >> %s",name, path, buf);
                fflush(stdout);
            }
        } else if(ch == 10) {
            // key enter
            break;
        } else {
            int i = strlen(buf);
            printf("%c", ch);
            buf[i] = ch; 
            i++;
        }
    }
    tcsetattr(0, TCSANOW, &old_opt); //recover
    printf("\n");
    memcpy(history_command[maxposition], buf, strlen(buf));   
    return buf;
}



char ** lsh_split_line(char* line){
    int position = 0;
    int bufsize = LSH_TOK_BUFSIZE;
    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;

    if(!tokens){
        fprintf(stderr, "lsh : allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL){
        tokens[position++] = token;
    
        if(position >= bufsize){
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, sizeof(char*) * bufsize);

            if(!tokens) {
                fprintf(stderr, "lsh : reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        //printf("split : %s\n", token);
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    
    return tokens;
}


int lsh_execute(char ** args) {
    
    if(args[0] == NULL) {
        // An empty command
        return 1;
    }

    for(int i = 0;i < lsh_num_builtins();++i) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    //printf("execute \n");
    return lsh_launch(args);
}

void lsh_loop(){
    char *line;
    char **args;
    history_count = 0;
    int status;

    char path[80];
    struct passwd *pwd;

    do {
        getcwd(path,sizeof(path));
        pwd = getpwuid(getuid());

        line = lsh_get_line(pwd->pw_name, path);
        history_count++; // up/down to history    

        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
        //free(pwd);
    } while(status);
    //free(pwd);
}


int main(int argc, char **argv){

    // Run command loop
    lsh_loop();

    return EXIT_SUCCESS;
}
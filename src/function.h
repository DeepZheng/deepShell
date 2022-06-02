#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define LSH_LINE_BUFSIZE 200
#define LSH_TOK_BUFSIZE 64
#define LSH_HIS_SIZE 100
#define LSH_TOK_DELIM  " \t\r\n\a"

//char history_command[LSH_HIS_SIZE][LSH_LINE_BUFSIZE];
char **history_command;
 int history_count;


char* builtin_str[] = {
    "cd",
    "help",
    "history",
    "exit"
};


int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args){
    if(args[1] == NULL) {
        fprintf(stderr, "lsh : expected argument to \"cd\" \n");
    } else {
        if(chdir(args[1]) != 0) {
            perror ("lsh");
        }
    }
    return 1;
}

int lsh_exit(char **args){
    printf("Bye~\n");
    return 0;
}

int lsh_help(char **args){
    printf("Welcome to deepz's LSH\n");
    printf("Type program name and argument, and hit enter\n");
    printf("The following are built in\n");

    for(int i = 0;i < lsh_num_builtins();i++){
        printf("\t%s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs\n");
    return 1;
}

int lsh_history(char **args){
    
    for(int i = 0;i < history_count; ++i){
        // don't use strlen(history_command[i]) != 0
        // in case some empty command input
        printf("%d \t%s\n", i,history_command[i]);
    }
    return 1;
}

int (*builtin_func[]) (char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_history,
    &lsh_exit,
};

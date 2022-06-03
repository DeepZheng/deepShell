#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include "builtin.h"
#include "profile.h"
#include "handle.h"

int cmd_add_to_history(char *line){
    /* initialize on first call */
	if (history_command == NULL) {
		history_command = calloc(sizeof(char *) * LSH_HIS_SIZE, 1);
		if (history_command == NULL) {
			fprintf(stderr, "error: allocation error\n");
			return 0;
		}
	}

    char *tmp = strdup(line);
    if(line == NULL) {
        fprintf(stderr, "error: allocation error\n");
        return 0;
    }

    /*

    */
    if(history_count == LSH_HIS_SIZE-1) {
        free(history_command[0]);
        // safer impletation than memcpy
        memmove(history_command, history_command+1, sizeof(char*) * (LSH_HIS_SIZE) - 1);

        if(history_command == NULL) {
            fprintf(stderr, "error: allocation error\n");
			return 0;
        }

        history_count--;
    }

    history_command[history_count++] = tmp;
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
    char *line = malloc(sizeof(char) * LSH_LINE_BUFSIZE);
    memset(line, 0, sizeof(char) * LSH_LINE_BUFSIZE);

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

                    // save current command
                    if(position == maxposition) {
                        strcpy(line, buf);
                    } 
                    // recover history command
                    if(position > 0) {
                        strcpy(buf, history_command[--position]);
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
                    if(position < maxposition-1){
                        strcpy(buf, history_command[++position]);
                    } else if(position == maxposition-1) {
                        strcpy(buf, line);
                        position++;
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
  
    cmd_add_to_history(buf);
    return buf;
}



char ** lsh_split_line(char* line){
    arg_length = 0;
    int bufsize = LSH_TOK_BUFSIZE;
    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;

    if(!tokens){
        fprintf(stderr, "error : allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL){
        tokens[arg_length++] = token;
    
        if(arg_length >= bufsize){
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, sizeof(char*) * bufsize);

            if(!tokens) {
                fprintf(stderr, "error : reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        //printf("split : %s\n", token);
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[arg_length] = NULL;
    
    return tokens;
}



/*
1. > : redirect output
2. < : redirect input
3. | : piping
3. normarl command
*/
int lsh_execute(char ** args) {
    int fd = -1;
    int start = 0;
    int success = 0;
    if(args[0] == NULL) {
        // An empty command
        return 1;
    }
    // check for builtin function
    for(int i = 0;i < lsh_num_builtins();++i) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    
    for(int i = 0;i < arg_length;i++) {

       if(strcmp(args[i], ">") == 0) {
           args[i] = NULL;
           fd = open(args[i+1], O_RDWR|O_CREAT, 0664);
           if(fd < 0) {
               perror("> open file error");
           }

           success = lsh_handle(args, -1, fd, start, REDIRECT_OUTPUT);
           fd = 0;
           i++;
       } else if (strcmp(args[i], "<") == 0) {
           args[i] = NULL;
           fd = open(args[i+1], O_RDONLY);
           if(fd < 0) {
               perror("< open file error");   
           }

           success = lsh_handle(args, fd, -1, start, REDIRECT_INPUT);
           fd = 0;
           i++;
       } else if (strcmp(args[i], "|") == 0) {
           args[i] = NULL;
           fd = lsh_handle(args, fd, -1, start, PIPE);
           start = i + 1;
       } 
       else if (i == arg_length-1){
           success = lsh_handle(args, fd, -1, start, NORMAL);
       }
   }
   return success;

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
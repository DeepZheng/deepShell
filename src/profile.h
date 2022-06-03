#ifndef PROFILE_H
#define PROFILE_H

#define LSH_LINE_BUFSIZE 200
#define LSH_TOK_BUFSIZE 64
#define LSH_HIS_SIZE 100
#define LSH_TOK_DELIM  " \t\r\n\a"

enum HANDLE_TYPE {NORMAL, REDIRECT_INPUT, REDIRECT_OUTPUT, PIPE};

char **history_command;
int history_count;

int arg_length;

#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>


void exe_cmd(char *argv_ptr[]) {
    pid_t pid;
    pid = fork();
    if(pid < 0) {
        perror("Error: Process creation failed!");
        exit(1);
    }
    else if(pid == 0) {
        execvp(argv_ptr[0],argv_ptr);
    }
    else {
        wait(NULL);
    }
}

void parse_cmd(char *cmd_str, char **argv) {
    char *tok;
    tok = strtok(cmd_str," \n");
    int token_cnt = 0;
    while(tok != NULL) {
        argv[token_cnt] = tok;
        tok = strtok(NULL, " \n");
        token_cnt++;
    }
    argv[token_cnt] = NULL;
}

int main(){

    char *argv[5];
    char commandBuf[100];
    while(1) {
        printf("ungal boodham >>");
        fgets(commandBuf,100,stdin);
        parse_cmd(commandBuf,argv);

        if(strcmp(argv[0],"exit")== 0) {
            printf("Application exiting...\n");
            break;
        }
        else {
        exe_cmd(argv);
        }  
    }
}

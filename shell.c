#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_ARGS 12
#define MAX_BUFF_SIZE 500

enum CMD_TYPE {TRIVIAL, REDIRECT, PIPING, BACKGROUND};
enum REDIR_TYPE {REDIR_IN, REDIR_OUT};

struct cmd {
    enum CMD_TYPE cmd_type;
    char **argv;
    struct redir_cmd *redir;
};


struct redir_cmd {
    enum REDIR_TYPE redir_type; // redirection type 
    char* file_path; // file path to redirect 
};


void handle_redirection(struct redir_cmd *rd) {
    int newfd;
    if (rd->redir_type == REDIR_OUT) {
        newfd = open(rd->file_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (newfd < 0) {
            perror(rd->file_path);
            exit(1);
        }
        dup2(newfd, STDOUT_FILENO);
    } else {
        newfd = open(rd->file_path, O_RDONLY);
        if (newfd < 0) {
            perror(rd->file_path);
            exit(1);
        }
        dup2(newfd, STDIN_FILENO);
    }

    close(newfd);
}


// i need to reuse the normal execution again in the redirection
void exe_cmd(struct cmd *cmd) {
    switch (cmd->cmd_type)
    {
    case TRIVIAL: 
        pid_t pid;
        pid = fork();
        if(pid < 0) {
            perror("Error: Process creation failed!");
            exit(1);
        }
        else if(pid == 0) {
            execvp(cmd->argv[0],cmd->argv);
            perror("Command execution failed");
            exit(1);

        }
        else {
            wait(NULL);
        }
        break;
    case REDIRECT:
        pid_t pid1;
        pid1 = fork();
        if(pid1< 0) {
            perror("Error: Process creation failed!");
            exit(1);
        }
        else if(pid1 == 0) {
            handle_redirection(cmd->redir);
            execvp(cmd->argv[0],cmd->argv);
            perror("Command execution failed");
            exit(1);

        }
        else {
            wait(NULL);
        }
    default:
        break;
    }

    
}


struct cmd *parse_cmd(char *cmd_str, char **argv) {
    struct cmd *cmd_struct = malloc(sizeof(struct cmd));

    struct redir_cmd *redir = NULL;

    char *tok;
    tok = strtok(cmd_str," \n");
    int tok_index = 0;
    while(tok != NULL) {
        if ((strcmp(tok, "<") == 0) || (strcmp(tok, ">") == 0)) {
            if (redir == NULL) {
                redir = malloc(sizeof(struct redir_cmd));
                redir->redir_type = (strcmp(tok, "<") == 0) ? REDIR_IN : REDIR_OUT;
            }
            tok = strtok(NULL, " \n"); 
            redir->file_path = tok;
        }

        else {
            argv[tok_index] = tok;
            tok_index++;

        }
        tok = strtok(NULL, " \n");

    }
    argv[tok_index] = NULL;
    cmd_struct->cmd_type = (redir != NULL) ? REDIRECT : TRIVIAL;
    cmd_struct->argv = argv;
    cmd_struct->redir = redir;
    return cmd_struct;
}

int main(){
    char commandBuf[MAX_BUFF_SIZE];
    char *argv[MAX_ARGS];
    struct cmd *cmd;
    while(1) {
        printf("Unix.Shell 0.1>>");
        fflush(stdout);
        fgets(commandBuf,MAX_BUFF_SIZE,stdin);
        if (strcmp(commandBuf,"\n") != 0) {
            cmd = parse_cmd(commandBuf, argv); 
            exe_cmd(cmd);
            free(cmd->redir);
            free(cmd);
        }
    }
}

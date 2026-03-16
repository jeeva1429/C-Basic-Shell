#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#define MAX_CMDS 10


struct cmd_exe_struct {
    char *args[50];     // arguments (like ls, -l, etc.)
    int is_redirected;  // 0 = no, 1 = yes
    char *redir_type;   // 1 = output (>), 0 = input (<)
    char *file_path;
};


struct pipeline {
    struct cmd_exe_struct cmds[MAX_CMDS];
    int cmd_count;
};

void execute_cmd(struct cmd_exe_struct cmd) {
    pid_t pid  = fork();
    if (pid < 0) {
        perror("Process creation failed!");
        exit(1);
    } else if (pid == 0) {
        if(cmd.is_redirected) {
            int file_fd;
            if(strcmp(cmd.redir_type, ">") == 0) {   // output ">"
                file_fd = open(cmd.file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(file_fd < 0) { perror("open failed"); exit(1); }
                dup2(file_fd, STDOUT_FILENO);
            } else {  // input "<"
                file_fd = open(cmd.file_path, O_RDONLY);
                if(file_fd < 0) { perror("open failed"); exit(1); }
                dup2(file_fd, STDIN_FILENO);
            }
            close(file_fd);
        }
        execvp(cmd.args[0], cmd.args);
        perror("Function execution failed!x");
        exit(1);
    }
    else {
        int status;
        waitpid(pid, &status,0);
    }
};

// cat < output.txt | sort | uniq | wc
int main() {
    char cmd_string[480];
    while (1) {
        struct pipeline *pipe_struct = calloc(1, sizeof(struct pipeline));
        int is_redirected = 0;
        printf("$");
        fgets(cmd_string, sizeof(cmd_string), stdin);
        cmd_string[strcspn(cmd_string,"\n")] = '\0';
        char *token = strtok(cmd_string, " ");
        if (strcmp(token, "cd") == 0) {
            printf("Feature is yet to be implemented\n");
        } else {
            int pipe_cnt= 0;
            int tokens_count = 0;
            while (token != NULL) {
                if (strcmp(token, "|") == 0) {
                    pipe_struct->cmds[pipe_cnt].args[tokens_count] = NULL;
                    pipe_cnt++;
                    tokens_count = 0;
                } else if (strcmp(token,">") == 0 || strcmp(token, "<") == 0) {
                    pipe_struct->cmds[pipe_cnt].is_redirected = 1;
                    pipe_struct->cmds[pipe_cnt].redir_type = token;
                } else {
                    if (pipe_struct->cmds[pipe_cnt].is_redirected && pipe_struct->cmds[pipe_cnt].file_path == NULL){
                        pipe_struct->cmds[pipe_cnt].file_path = token;
                    } else {
                        pipe_struct->cmds[pipe_cnt].args[tokens_count] = token;
                    }
                    tokens_count++;
                }
                token = strtok(NULL, " ");
            };
            pipe_struct->cmd_count = pipe_cnt+1;
            pipe_struct->cmds[pipe_cnt].args[tokens_count] = NULL;

            if (pipe_cnt == 0) {
                execute_cmd(pipe_struct->cmds[0]);
            } else {
                int pipeFd[pipe_cnt][2];
                for (int i = 0; i < pipe_cnt; i++) {
                    if (pipe(pipeFd[i]) == -1) {
                           perror("pipe initialization failed");
                           exit(1);
                    }
                };
                for (int i = 0; i < pipe_struct->cmd_count; i++) {
                    printf("cmd[%d]: args[0] = %s\n", i, pipe_struct->cmds[i].args[0]);
                }
                pid_t pids[MAX_CMDS];
                for(int i = 0; i < pipe_struct->cmd_count; i++) {
                    pids[i] = fork();
                    if (pids[i] < 0) {
                        perror("Process creation failed!");
                        exit(1);
                    } else if (pids[i] == 0) {
                        if (i < pipe_cnt) {
                            dup2(pipeFd[i][1], STDOUT_FILENO);
                        }
                        if (i > 0) {
                            dup2(pipeFd[i-1][0], STDIN_FILENO);
                        }
                        if(pipe_struct->cmds[i].is_redirected) {
                            int file_fd;
                            if(strcmp(pipe_struct->cmds[i].redir_type, ">") == 0) {   // output ">"
                                file_fd = open(pipe_struct->cmds[i].file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                if(file_fd < 0) { perror("open failed"); exit(1); }
                                dup2(file_fd, STDOUT_FILENO);
                            } else {  // input "<"
                                file_fd = open(pipe_struct->cmds[i].file_path, O_RDONLY);
                                if(file_fd < 0) { perror("open failed"); exit(1); }
                                dup2(file_fd, STDIN_FILENO);
                            }
                            close(file_fd);
                        }
                        for(int j = 0; j < pipe_cnt; j++) {
                            close(pipeFd[j][0]);
                            close(pipeFd[j][1]);
                        }
                        execvp(pipe_struct->cmds[i].args[0], pipe_struct->cmds[i].args);
                        perror("command execution failed!");
                        exit(1);
                    }
                }

                for(int j = 0; j < pipe_cnt; j++) {
                    close(pipeFd[j][0]);
                    close(pipeFd[j][1]);
                }

                for (int i = 0; i < pipe_struct->cmd_count; i++) {
                    waitpid(pids[i], NULL, 0);
                }
            }
        }
        free(pipe_struct); //free the allocated memory for command arguments
    }
}

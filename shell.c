#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

struct cmd_exe_struct {
    char *args[20];     // arguments (like ls, -l, etc.)
    int is_redirected;  // 0 = no, 1 = yes
    int redir_type;     // 1 = output (>), 0 = input (<)
    char *file_path;    // file for redirection
};

void execute_cmd(struct cmd_exe_struct *cmd) {
    pid_t pid  = fork();
    if (pid < 0) {
        perror("Process creation failed!");
        exit(0);
    } else if (pid == 0) {
        if(cmd->is_redirected) {
        int file_fd;
        if(cmd->redir_type == 1) {   // output ">"
            file_fd = open(cmd->file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(file_fd < 0) { perror("open failed"); exit(1); }
            dup2(file_fd, STDOUT_FILENO);
        } else {                     // input "<"
            file_fd = open(cmd->file_path, O_RDONLY);
            if(file_fd < 0) { perror("open failed"); exit(1); }
            dup2(file_fd, STDIN_FILENO);
        }
        close(file_fd);  // close original FD after dup2
        }
        execvp(cmd->args[0], cmd->args);
    }
    else {
        int status;
        waitpid(pid, &status,0);
    }
};

// cat < output.txt | sort | uniq | wc
int main() {
    char cmd_string[48];
    int command_size = 10;
    struct cmd_exe_struct *cmd_exe_ptr = malloc(sizeof(struct cmd_exe_struct));
    while (1) {
        int is_redirected = 0;
        printf("$");
        fgets(cmd_string, sizeof(cmd_string), stdin);
        char *cmd_arr[command_size];
        cmd_string[strcspn(cmd_string,"\n")] = '\0';
        char *token = strtok(cmd_string, " ");
        int tokens_count = 0;
        if (strcmp(token, "cd") == 0) {
            printf("Feature is yet to be implemented\n");
        } else {
            while(token != NULL) {
                if (strcmp(token,">") == 0 || strcmp(token, "<") == 0) {
                    is_redirected = 1;
                    break;
                }
                cmd_exe_ptr->args[tokens_count] = token;
                tokens_count ++;
                token = strtok(NULL," ");
            }
            cmd_exe_ptr->args[tokens_count] = NULL;
            cmd_exe_ptr->is_redirected = 0;
            if(is_redirected == 1) {
                int redir_type = strcmp(token,">") == 0 ? 1 : 0;
                char* file_path =  strtok(NULL," ");
                if (file_path == NULL) {
                    perror("Please input valid file path to redirect");
                    exit(1);
                };
                cmd_exe_ptr->redir_type = redir_type;
                cmd_exe_ptr->file_path = file_path;
                cmd_exe_ptr->is_redirected = 1;
            }
            execute_cmd(cmd_exe_ptr);
            free(cmd_exe_ptr); //free the allocated memory for command arguments
        }
    }
    
}

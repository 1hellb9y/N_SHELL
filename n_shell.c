#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

int parse_command(
    char *args[],
    int start,
    int end,
    char *cmd_args[],
    char **input_file,
    char **output_file,
    int *append_file
)
{
    int count = 0;

    *input_file = NULL;
    *output_file = NULL;
    *append_file = 0;

    for (int i = start; i < end; i++) {

        /* Input redirection */
        if (strcmp(args[i], "<") == 0) {

            if (i + 1 >= end) {
                printf("syntax error: expected filename after <\n");
                return -1;
            }

            *input_file = args[++i];
        }

        /* Output redirection */
        else if (strcmp(args[i], ">") == 0) {

            if (i + 1 >= end) {
                printf("syntax error: expected filename after >\n");
                return -1;
            }

            *output_file = args[++i];
            *append_file = 0;
        }

        /* Append redirection */
        else if (strcmp(args[i], ">>") == 0) {

            if (i + 1 >= end) {
                printf("syntax error: expected filename after >>\n");
                return -1;
            }

            *output_file = args[++i];
            *append_file = 1;
        }

        /* Normal command argument */
        else {
            cmd_args[count++] = args[i];
        }
    }

    cmd_args[count] = NULL;

    return count;
}


void redirect_input(char *filename)
{
    if (filename == NULL) {
        return;
    }

    int fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("open");
        _exit(1);
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
        perror("dup2");
        close(fd);
        _exit(1);
    }

    close(fd);
}


void redirect_output(char *filename, int append)
{
    if (filename == NULL) {
        return;
    }

    int flags = O_WRONLY | O_CREAT;

    if (append) {
        flags |= O_APPEND;
    }
    else {
        flags |= O_TRUNC;
    }

    int fd = open(filename, flags, 0644);

    if (fd == -1) {
        perror("open");
        _exit(1);
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("dup2");
        close(fd);
        _exit(1);
    }

    close(fd);
}


int main(void)
{   
    signal(SIGINT,SIG_IGN);
    while (1) {

        char buffer[1024];

        char *args[100];
        int argc = 0;

        printf("N-shell> ");
        fflush(stdout);

        /* Read command */
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        /* Remove newline */
        buffer[strcspn(buffer, "\n")] = '\0';

        /* Tokenize command */
        char *token = strtok(buffer, " \t");

        while (token != NULL && argc < 99) {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }

        args[argc] = NULL;

        if (argc == 0) {
            continue;
        }


        /* Find pipe */
        int pipe_index = -1;

        for (int i = 0; i < argc; i++) {

            if (strcmp(args[i], "|") == 0) {
                pipe_index = i;
                break;
            }
        }


        /*
         * ==================================================
         * NO PIPE
         * ==================================================
         */

        if (pipe_index == -1) {

            char *cmd_args[100];

            char *input_file;
            char *output_file;

            int append_file;

            int cmd_argc = parse_command(
                args,
                0,
                argc,
                cmd_args,
                &input_file,
                &output_file,
                &append_file
            );

            if (cmd_argc <= 0) {
                continue;
            }


            /*
             * Built-in exit
             */

            if (strcmp(cmd_args[0], "exit") == 0) {
                break;
            }


            /*
             * Built-in cd
             */

            if (strcmp(cmd_args[0], "cd") == 0) {

                if (cmd_args[1] == NULL) {
                    printf("cd: missing directory\n");
                }
                else if (chdir(cmd_args[1]) == -1) {
                    perror("cd");
                }

                continue;
            }


            /*
             * Create child
             */

            pid_t pid = fork();

            if (pid < 0) {
                perror("fork");
                continue;
            }


            /*
             * Child process
             */

            if (pid == 0) {
                signal(SIGINT,SIG_DFL);
                redirect_input(input_file);

                redirect_output(
                    output_file,
                    append_file
                );

                execvp(cmd_args[0], cmd_args);

                perror("execvp");

                _exit(1);
            }
                      int status;
            waitpid(pid,&status,0);
            if(WIFSIGNALED(status)  && WTERMSIG(status)==SIGINT){
                printf("\n");
 
            }

            }


        /*
         * ==================================================
         * PIPE
         * ==================================================
         */

        else {

            if (pipe_index == 0 || pipe_index == argc - 1) {
                printf("syntax error: invalid pipe\n");
                continue;
            }


            char *left_args[100];
            char *right_args[100];

            char *left_input;
            char *left_output;
            int left_append;

            char *right_input;
            char *right_output;
            int right_append;


            /*
             * Parse left side
             */

            int left_count = parse_command(
                args,
                0,
                pipe_index,
                left_args,
                &left_input,
                &left_output,
                &left_append
            );


            /*
             * Parse right side
             */

            int right_count = parse_command(
                args,
                pipe_index + 1,
                argc,
                right_args,
                &right_input,
                &right_output,
                &right_append
            );


            if (left_count <= 0 || right_count <= 0) {
                continue;
            }


            /*
             * Create pipe
             */

            int fd[2];

            if (pipe(fd) == -1) {
                perror("pipe");
                continue;
            }


            /*
             * ==================================================
             * LEFT PROCESS
             * ==================================================
             */

            pid_t pid1 = fork();

            if (pid1 < 0) {

                perror("fork");

                close(fd[0]);
                close(fd[1]);

                continue;
            }


            if (pid1 == 0) {

                signal(SIGINT, SIG_DFL);
                redirect_input(left_input);


                /*
                 * Send stdout into pipe
                 */

                if (dup2(fd[1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    close(fd[0]);
                    close(fd[1]);
                    _exit(1);
                }


                close(fd[0]);
                close(fd[1]);


                /*
                 * If left command has explicit
                 * output redirection, it replaces
                 * the pipe output.
                 */

                if (left_output != NULL) {

                    redirect_output(
                        left_output,
                        left_append
                    );
                }


                execvp(left_args[0], left_args);

                perror("execvp");

                _exit(1);
            }


            /*
             * ==================================================
             * RIGHT PROCESS
             * ==================================================
             */

            pid_t pid2 = fork();

            if (pid2 < 0) {

                perror("fork");

                close(fd[0]);
                close(fd[1]);

                waitpid(pid1, NULL, 0);

                continue;
            }


            if (pid2 == 0) {

                signal(SIGINT, SIG_DFL);
                if (dup2(fd[0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    close(fd[0]);
                    close(fd[1]);
                    _exit(1);
                }


                close(fd[0]);
                close(fd[1]);


                /*
                 * Explicit input redirection
                 * replaces pipe input.
                 */

                if (right_input != NULL) {
                    redirect_input(right_input);
                }


                /*
                 * Output redirection
                 */

                redirect_output(
                    right_output,
                    right_append
                );


                execvp(right_args[0], right_args);

                perror("execvp");

                _exit(1);
            }


            /*
             * Parent doesn't use pipe descriptors
             */

            close(fd[0]);
            close(fd[1]);


            /*
             * Wait for both processes
             */

            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }


    return 0;
}
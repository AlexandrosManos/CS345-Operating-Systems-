/* Alexandros Manos*/
/*      csd5136    */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "global_var.h"
#include "input_output.h"
#include "execution.h"

int split_len(char **split_inp);
void print_command(char **command);

int main(int argc, char **argv)
{
    pid_t pid;
    int i, count;
    char *shell_name, *input = NULL, inputCopy[LINE_SIZE], **pipe_input[MPIPES] = {0}, **split_inp = NULL;
    shell_name = set_name();
    printf("%s ", shell_name);
    printf("\n");
    while (1)
    {
        input = split_command();
        if (input == NULL)
        {
            continue;
        }
        strcpy(inputCopy, input);
        /*printf("Copy = <%s>\n", inputCopy);*/
        split_inp = token_split(input);
        if (strcmp(split_inp[0], "exit") == 0)
        {
            printf("\033[0;31mExit...\033[0;37m\n");
            break;
        }
        else
        {
            /*If there are any pipes*/
            if (state == PIPE)
            {
                count = 0;
                pipe_input[count++] = split_inp;

                while (state == PIPE)
                {
                    input = split_command();
                    if (input == NULL)
                    {
                        break;
                    }
                    split_inp = token_split(input);
                    pipe_input[count++] = split_inp;
                }
                exe_pipe(pipe_input, count);
            } /*For globals*/
            else if (strchr(split_inp[0], '=') != NULL)
            {
                create_global(inputCopy);
            }
            else
            {
                pid = fork();
                if (pid == -1)
                {
                    fprintf(stderr, "Could not create a child process!\n");
                    break;
                }
                if (pid == 0) /* Child Process */
                {

                    for (i = 0; split_inp[i] != NULL; i++)
                    {
                        count = command_exe(split_inp);
                    }
                    exit(0);
                }
                else
                    wait(NULL);
            }
        }

        free(input);
        free(split_inp);
        input = NULL;
        split_inp = NULL;
    }
    free(input);
    free(split_inp);
    free(shell_name);
    return 0;
}

int split_len(char **split_inp)
{
    int count;
    count = 0;
    while (split_inp[count] != NULL)
    {
        count++;
    }
    return count;
}

void print_command(char **command)
{
    int i = 0;
    while (command[i] != NULL)
    {
        printf("Token %d: %s\n", i, command[i]);
        i++;
    }
    printf("\n");
}
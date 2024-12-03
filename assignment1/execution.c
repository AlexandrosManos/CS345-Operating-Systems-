#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "execution.h"

#include "global_var.h"

int command_exe(char **cmd)
{
    int fin = -1, fout = -1, i;
    if (strncmp(cmd[0], "echo", 4) == 0)
    {
        i = 1;
        while (cmd[i] != NULL)
        {
            if (strlen(cmd[i]) > 1)
            {
                /* Replace the name of the variable with its value */
                echo(&cmd[i]);
            }
            i++;
        }
    }
    for (i = 0; cmd[i] != NULL; i++)
    {
        /* There is no such a command with double "<" */
        if (strcmp(cmd[i], "<") == 0)
        {
            cmd[i++] = NULL;
            fin = open(cmd[i], O_RDONLY);
            if (fin < 0)
            {
                perror("Could not open the input file");
                return -1; /*Failure*/
            }
            /* Redirect stdin */
            dup2(fin, STDIN_FILENO);
            close(fin);
        } /* Using strncmp in case of double redirection */
        else if (strncmp(cmd[i], ">", 1) == 0)
        {
            if (strcmp(cmd[i], ">") == 0)
            {
                /*O_TRUNC --> Overwrite*/
                /*rw for user, r for group and others*/
                fout = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            else
            {
                fout = open(cmd[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            }
            cmd[i++] = NULL;
            if (fout < 0)
            {
                perror("Could not open the output file");
                return -1; /*Failure*/
            }
            /* Rediracting stdout */
            dup2(fout, STDOUT_FILENO);
            close(fout);
        }
    }

    /* Execute the command */
    if (execvp(cmd[0], cmd) == -1)
    {
        perror("Unknown command");
        return -1; /*Failure*/
    }
    return 1; /*Success*/
}

void exe_pipe(char ***cmds, int count)
{
    /* Count Pipes*/
    int pipes[count - 1][2];
    int i, j;
    pid_t pid;

    if (cmds == NULL)
    {
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < count; i++)
    {
        if (i < count - 1)
        {
            if (pipe(pipes[i]) < 0)
            {
                perror("Failed to create pipe");
                exit(EXIT_FAILURE);
            }
        }
        pid = fork();
        if (pid < 0)
        {
            perror("Child process failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) /* Child Process */
        {
            if (i != 0) /*We do not need to redirect the stdin for the first command*/
            {
                /* Redirect stdin */
                if (dup2(pipes[i - 1][0], STDIN_FILENO) < 0)
                {
                    perror("Could not redirect the input");
                    exit(EXIT_FAILURE);
                }
            }
            /* More than one command left */
            /*We do not need to redirect the stdout on the last command*/
            if (i < count - 1)
            {
                /* Redirect stdout */
                if (dup2(pipes[i][1], STDOUT_FILENO) < 0)
                {
                    perror("Could not redirect the input");
                    exit(EXIT_FAILURE);
                }
            }
            /* Close pipes */
            for (j = 0; j < count - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            if (command_exe(cmds[i]) < 0)
            {
                exit(EXIT_FAILURE);
            }
        }
    }
    for (i = 0; i < count - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    /* Wait for all the processes to finish */
    while (1)
    {
        if (wait(NULL) <= 0)
        {
            break;
        }
    }
    return;
}

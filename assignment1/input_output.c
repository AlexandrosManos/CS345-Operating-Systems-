#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "input_output.h"

enum states state;

/* LINE_SIZE + 1 for \0 */
static char command_line[LINE_SIZE + 1]; /* Store the input line */
static int index = 0;

char *set_name()
{
    char am[] = "csd5136";
    char *user = getlogin();
    char *abs_path = malloc(PATH_SIZE * sizeof(char));
    char *shell_name = malloc(NSHELL_SIZE * sizeof(char));

    if (shell_name == NULL)
    {
        printf("Failed to allocate memory\n");
        exit(-1);
    }
    if (abs_path == NULL)
    {
        printf("Failed to allocate memory\n");
        exit(-1);
    }
    getcwd(abs_path, PATH_SIZE);
    if (abs_path == NULL)
    {
        exit(-1);
    }
    if (user == NULL)
    {
        user = " ";
    }

    sprintf(shell_name, "<\033[0;31m%s\033[0;37m>-\033[0;35mhy345sh@\033[0;37m<\033[0;32m%s\033[0;37m>:<\033[0;34m%s\033[0;37m> ", am, user, abs_path);
    free(abs_path);
    return shell_name;
}

/* Read and return the input line*/
char *get_line()
{
    char *input = malloc(LINE_SIZE * sizeof(char));
    printf(">");
    if (fgets(input, LINE_SIZE, stdin) != NULL)
    {
        /*remove the \n and replace it with \0*/
        input[strcspn(input, "\n")] = 0;
    }
    else
    {
        input = "\0";
    }
    return input;
}

/* Split the whole command line into commands based on semicolon, pipes or EOF */
char *split_command()
{
    /* The returning command */
    char *line;
    char c;
    int command_index = 0;

    if (state == SIMPLE)
    {
        strncpy(command_line, get_line(), LINE_SIZE);
        index = 0;
    }
    /* Plus one for \0 */
    line = malloc((strlen(&(command_line[index])) + 1) * sizeof(char));
    while (1)
    {
        c = command_line[index++];

        /* Remove extra spaces */
        if (c == ' ')
        {
            if (command_index != 0)
                line[command_index++] = c;
            while ((c = command_line[index]) == ' ')
                index++;
            continue;
        }
        else if (c == '<' || c == '>')
        {
            if (command_index != 0)
            {
                /* Add 1 space before and after */
                if (line[command_index - 1] != ' ' && line[command_index - 1] != c)
                    line[command_index++] = ' ';
            }
            line[command_index++] = c;
            if (command_line[index] != ' ' && command_line[index] != c)
                line[command_index++] = ' ';
        }
        else if (c == '\"')
        {
            line[command_index++] = c;
            if (command_index > 1 && line[command_index - 2] == '=')
            {
                do
                {
                    c = command_line[index++];
                    if (index <= LINE_SIZE)
                    {
                        line[command_index++] = c;
                    }
                    else
                    {
                        break;
                    }
                } while (c != '\"');
            }
        }
        else if (c == '\n' || c == ';' || c == '\0' || c == EOF || c == '|')
        {
            change_state(c);
            /* Remove the last space */
            if (command_index > 0 && line[command_index - 1] == ' ')
                command_index--;
            /* End of the string */
            line[command_index] = '\0';
            if (command_index == 0)
            {
                free(line);
                return NULL;
            }
            return line;
        }
        else
        {
            if (index <= LINE_SIZE)
            {
                line[command_index++] = c;
            }
            else
            {
                perror("The command line is way too big");
            }
        }
    }
    assert(0);
}

void change_state(char c)
{
    switch (c)
    {
    case '|':
        state = PIPE;
        break;
    case ';':
        state = SEMI;
        break;
    default:
        state = SIMPLE;
        break;
    }
}

char *print_state(enum states state)
{
    if (state == PIPE)
    {
        return "Pipe state";
    }
    else if (state == SEMI)
    {
        return "Semicolon state";
    }
    else
    {
        return "Simple state";
    }
}

/* Split the command into tokens, return an array of "strings" */
char **token_split(char *command)
{
    /* Up to 199 arguments */
    char **tokens = malloc(200 * sizeof(char *));
    char *token;
    int i = 0;
    if (tokens == NULL)
    {
        printf("Malloc Failed\n");
        exit(EXIT_FAILURE);
    }

    /* get the first token */
    token = strtok(command, " ");

    /* walk through other tokens */
    while (token != NULL)
    {
        tokens[i++] = token;
        if (i == 199)
            break;
        token = strtok(NULL, " ");
    }
    tokens[i] = NULL;
    return tokens;
}

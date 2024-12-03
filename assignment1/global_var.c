#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input_output.h"
#include "global_var.h"

#define LINE_SIZE 1024

void echo(char **input)
{
    char *output;
    char *token = strtok(*input, " $");
    char *value;
    output = malloc(LINE_SIZE * sizeof(char));
    if (output == NULL)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    /* Initialize string with NULL-terminated strings */
    memset(output, 0, LINE_SIZE);
    if (*input[0] == '$')
    {

        while (token != NULL)
        {
            value = getenv(token);
            if (value != NULL)
            {
                strcat(output, value);
            }
            else
            {
                /* Concatenates the token and the output */
                strcat(output, token);
            }
            token = strtok(NULL, " $");
        }
    }
    else
    {
        return;
    }
    /* Print the global variable */
    *input = output;
}

void create_global(char *input)
{
    char delim[5];
    char *next_part;
    char *var_name;
    char *var_value;

    /*remove the \n and replace it with \0*/
    input[strcspn(input, "\n")] = '\0';

    /* Create new global variable */
    if (strchr(input, '=') != NULL)
    {
        /* Check if the next character, after the "=", is \" */
        next_part = strchr(input, '=');
        if (next_part != NULL)
        {
            next_part++;
            if (next_part != NULL && strncmp(next_part, "\"", 1) == 0)
            {
                strcpy(delim, "\"");
            }
            else
            {
                strcpy(delim, "=;| ");
            }
        }
        var_name = strtok(input, "=");
        var_value = strtok(NULL, delim);
        if (var_value == NULL)
        {
            return;
        }
        if (setenv(var_name, var_value, 1) != 0)
        {
            perror("Failed to create global variable");
        }
    }
    else
    {
        perror("Unknown command\n");
    }

    return;
}

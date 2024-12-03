#define PATH_SIZE 500
#define NSHELL_SIZE 1024
#define LINE_SIZE 1024

enum states
{
    SIMPLE = 0,
    SEMI = 1,
    PIPE = 2

};

char *set_name();
char *get_line();
char *split_command();
void change_state(char c);
char *print_state(enum states state);
char **token_split(char *command);

extern enum states state;

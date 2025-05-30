#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define HELP_DATA \
"NAME:\n"\
"\tspwd - Short Print Working Directory\n\n"\
"DESCRIPTION:\n"\
"\tPrint the name of the current working directory, \n"\
"\tshortening where possible to match the required output width\n\n"\
"USAGE:\n"\
"\tspwd [arguments]\n\n"\
"ARGUMENTS:\n"\
"\t-w, --width <width>\n"\
"\t\tOutput width. Uses the terminal width, if not declared\n"\
"\t-s, --subtract <number>\n"\
"\t\tSubtract the required number of characters from the output width\n"\
"\t-L, --logical\n"\
"\t\tUse the PWD environment variable, even if it contains symlinks\n"\
"\t-P, --physical\n"\
"\t\tPrint the directory name without symlinks\n"\
"\t-h, --help\n"\
"\t\tDisplay this help and exit"

/**
 * The path to be printed
 */
const char* path = NULL;

/**
 * The max lenght of the output
 */
int max_width = 0;

/**
 * @return The terminal width, which can be used to print the directory
 */
int get_terminal_width(void);

/**
 * @return The current working directory (with symlinks)
 */
const char* get_logical_dir(void);

/**
 * @return The physical path to the current working directory (ignore symlinks)
 */
const char* get_physical_dir(void);

/**
 * Handle passed arguments
 * @param arg_ptr The pointer to the first argmuent string
 * @param arg_end The pointer to the last argmuent string
 */
void handle_args(char const* const* arg_ptr, char const* const* const arg_end);

/*
 * Print the current working directory
 */
void print_working_dir(void);

int main(const int arg_c, char const* const* arg_ptr) {
    /* Get the terminal width and set it as max width value */
    max_width = get_terminal_width();

    /* Handle arguments (if exist) */
    if (arg_c > 1) handle_args(arg_ptr, arg_ptr + arg_c);

    /* Format and print the current working directory */
    print_working_dir();
    return 0;
}

int get_terminal_width() {
    /* Check all file descriptors */
    for (int fd = STDIN_FILENO; fd <= STDERR_FILENO; fd++) {
        /* If this descryptor is not a tty, get the next one */
        if (!isatty(fd)) continue;

        /* Get the terminal width */
        struct winsize w;
        ioctl(fd, TIOCGWINSZ, &w);

        /* If the path var is not definied yet, put the logical dir there */
        if (path == NULL) path = get_logical_dir();

        /* Return the value */
        return w.ws_col;
    }

    /* If it's impossible to get the width */
    return 0;
}

const char* get_physical_dir() {
    return getcwd(0, 0);
}

const char* get_logical_dir() {
    return getenv("PWD");
}

void handle_args(char const* const* arg_ptr, char const* const* const arg_end) {
    /* Check all arguments */
    while (++arg_ptr < arg_end) {
        /* Get flags only */
        if ((*arg_ptr)[0] != '-') continue;

        /* Handle flag according to its type ('-' or '--')) */
        if ((*arg_ptr)[1] == '-') {  // Flag type: '--'
            if (strcmp(*arg_ptr + 2, "width") == 0 && arg_ptr + 1 != arg_end)
                max_width = atoi(*(arg_ptr + 1));  // Width argument
            else if (strcmp(*arg_ptr + 2, "subtract") == 0 && arg_ptr + 1 != arg_end)
                max_width -= atoi(*(arg_ptr + 1));  // Subtract argument
            else if (strcmp(*arg_ptr + 2, "logical") == 0) path = get_logical_dir();  // Logical argument
            else if (strcmp(*arg_ptr + 2, "physical") == 0) path = get_physical_dir();  // Physical argument
            else if (strcmp(*arg_ptr + 2, "help") == 0) {  //Help argument
                printf(HELP_DATA);
                exit(0);
            }
            continue;
        }

        /* Flag type: '-' */
        switch ((*arg_ptr)[1]) {
            case 'w':  // Width argument
                /* Get the output width if exists */
                if (arg_ptr + 1 != arg_end)
                    max_width = atoi(*(arg_ptr + 1));
                continue;
            case 's':  // Subtract argument
                /* Subtract the number of chars if exists */
                if (arg_ptr + 1 != arg_end)
                    max_width -= atoi(*(arg_ptr + 1));
                continue;
            case 'L':  // Logical argument
                path = get_logical_dir();
                continue;
            case 'P':  // Physical argument
                path = get_physical_dir();
                continue;
            case 'h':  // Help argument
                printf(HELP_DATA);
                exit(0);
        }
    }
}

void print_working_dir() {
    /* Get the nesting level */
    int nest_level = 0;
    for (const char* path_ptr = path; *path_ptr != '\0'; path_ptr++)
        if (*path_ptr == '/') nest_level++;

    /* If we have very few directories, print the original data and exit */
    if (nest_level <= 3) {
        printf(path);
        return;
    }

    /* Get the current directory size */
    int path_sz = strlen(path);

    /* Get the difference between the maximum output width and the directory size */
    int size_delta = path_sz - max_width;

    /* If the size of the current directory is less than the maximum, just print it */
    if (size_delta <= 0) {
        printf(path);
        return;
    }

    /* Get the number of characters to replace to "/..." */
    size_delta += 4;

    /* Get the pointer to the replaceble part of the path */
    char* const path_replaceable = strchr(strchr(path + 1, '/') + 1, '/');
    char* path_replaceable_ptr = path_replaceable;

    /* Get the pointer to the last immutable part of the path */
    char* const path_end = strrchr(path_replaceable_ptr, '/');

    /* Decalare vars for the replace pointer and the replace size */
    char* replace_part = NULL;
    int replace_size = 2147483647; // Int max

    /* Cycle through all possible ways to shorten the path */
    do {
        /* Get pointer to the first possible shortening end */
        char const* const path_replaceable_end = strchr(path_replaceable_ptr + size_delta, '/');

        /* If the pointer is too big, there isn't any shortenings anymore */
        if (path_replaceable_end == NULL || path_replaceable_end > path_end) break;

        /* Eval the current replacable part size */
        const int cur_size = path_replaceable_end - path_replaceable_ptr;

        /* If it is the worse way, continue finding */
        if (cur_size >= replace_size) continue;

        /* If it is the better way, replace the old one */
        replace_part = path_replaceable_ptr;
        replace_size = cur_size;
    } while ((path_replaceable_ptr = strchr(path_replaceable_ptr + 1, '/')) < path_end);

    /* Print the result */
    if (replace_part == NULL) {
        path_replaceable[0] = '\0';
        printf("%s/...%s", path, path_end);
    }
    else {
        replace_part[0] = '\0';
        printf("%s/...%s", path, replace_part + replace_size);
    }
}

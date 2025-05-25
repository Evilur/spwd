#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

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

const char* path = NULL;
bool physical_pwd = false;
int max_width = 0;

/* Get the terminal width and use it as a default width */
void set_max_width() {
    /* Check all file descriptors */
    for (int fd = STDIN_FILENO; fd <= STDERR_FILENO; fd++) {
        /* If this descryptor is not a tty, get the next one */
        if (!isatty(fd)) continue;

        /* Get the terminal width */
        struct winsize w;
        ioctl(fd, TIOCGWINSZ, &w);

        /* Set the value and exit */
        max_width = w.ws_col;
        return;
    }
}

void handle_args(char const* const* arg_ptr, char const* const* const arg_end) {
    /* Check all arguments */
    while (++arg_ptr < arg_end) {
        /* Get flags only */
        if ((*arg_ptr)[0] != '-') continue;

        /* Handle flag according to its type ('-' or '--')) */
        if ((*arg_ptr)[1] == '-') {  //Flag type: '--'
            if (strcmp(*arg_ptr + 2, "width") == 0 && arg_ptr + 1 != arg_end)
                max_width = atoi(*(arg_ptr + 1));  //Width argument
            else if (strcmp(*arg_ptr + 2, "subtract") == 0 && arg_ptr + 1 != arg_end)
                max_width -= atoi(*(arg_ptr + 1));  //Subtract argument
            else if (strcmp(*arg_ptr + 2, "logical") == 0) physical_pwd = false;  //Logical argument
            else if (strcmp(*arg_ptr + 2, "physical") == 0) physical_pwd = true;  //Physical argument
            else if (strcmp(*arg_ptr + 2, "help") == 0) {  //Help argument
                printf(HELP_DATA);
                exit(0);
            }
            continue;
        }

        /* Flag type: '-' */
        switch ((*arg_ptr)[1]) {
            case 'w':  //Width argument
                /* Get the output width if exists */
                if (arg_ptr + 1 != arg_end)
                    max_width = atoi(*(arg_ptr + 1));
                continue;
            case 's':  //Subtract argument
                /* Subtract the number of chars if exists */
                if (arg_ptr + 1 != arg_end)
                    max_width -= atoi(*(arg_ptr + 1));
                continue;
            case 'L':  //Logical argument
                physical_pwd = false;
                continue;
            case 'P':  //Physical argument
                physical_pwd = true;
                continue;
            case 'h':  //Help argument
                printf(HELP_DATA);
                exit(0);
        }
    }
}

void set_physical_dir() { path = getcwd(0, 0); }

void set_logical_dir() { path = getenv("PWD"); }

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
    int replace_size = 2147483647;

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

int main(const int arg_c, char const* const* arg_ptr) {
    /* Set the output width */
    set_max_width();

    /* Handle arguments (if exist) */
    if (arg_c > 1) handle_args(arg_ptr, arg_ptr + arg_c);

    /* Set the current working directory */
    if (physical_pwd) set_physical_dir();
    else set_logical_dir();

    /* Format and print the current working directory */
    print_working_dir();
    return 0;
}

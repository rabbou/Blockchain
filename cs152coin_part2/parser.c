#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"

static const size_t LINE_MAX = 2048;
static const char* SEP = " \t\n";

static char* token = NULL;
static char buffer[LINE_MAX];

void init_parser()
{
    next_token();
}

void next_token()
{
    if (token != NULL) {
        token = strtok(NULL, SEP);
    }
    
    while (token == NULL) {
        // leave room so that we can insert extra space before the newline
        char* s = fgets(buffer, LINE_MAX, stdin);
        if (s == NULL) {
            if (ferror(stdin)) {
                fprintf(stderr, "Error reading input\n");
                exit(1);
            }
            assert(feof(stdin));
            break;
        }
        
        token = strtok(s, SEP);
    }
}

int read_string(char** s)
{
    *s = token;
    return token != NULL;
}

int read_double(double* d)
{
    if (token == NULL) return 0;

    sscanf(token, "%lf", d);
    return 1;
}
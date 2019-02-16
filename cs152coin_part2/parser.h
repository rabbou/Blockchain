#ifndef _PARSER_H
#define _PARSER_H

// Initialize the parser so that it's ready to go
void init_parser();

// Advance to the next token in the input
void next_token();

// Read the current token as a string and store a pointer to it in s.
// The string that is returned does not need to be freed, 
// but it will be overwritten on the next call to next_token()
//
// Return 0 if no token could be read because end of the file was reached,
// and nonzero otherwise.
int read_string(char** s);

// Read the current token as a double and store it in d.
//
// Return 0 if no token could be read because end of the file was reached,
// and nonzero otherwise.
int read_double(double* d);


#endif
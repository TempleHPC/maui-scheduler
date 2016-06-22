
#include "maui_utils.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/** Duplicate a string
 *
 * This function will take the string passed as argument
 * create a copy of it in newly allocated memory. The new
 * memory is allocated with malloc() to a suitable size.
 * The function returns a pointer to that copy which can
 * be passed to free() in order to return the allocated
 * memory to the system.
 *
 * @param input string to be copied
 * @return pointer to the new storage with the copy of
 * the input string or NULL if the call to malloc failed.
 */

char *string_dup(const char *input){

    char *output;
    int len;

    len = strlen(input) + 1;
    output = (char *) malloc(len);
    if (output) strcpy(output,input);
    return output;
}

/** Convert string to integer
 *
 * This function will take the string passed as argument,
 * check if it is a valid number and then convert it to
 * and integer. If it is not a number the value INVALID_STRING
 * is returned.
 *
 * @param input string to be converted
 * @return value of string converted to integer or INVALID_STRING.
 */

int string2int(const char *input){

    int i,n;

    if (input == NULL) return INVALID_STRING;

    n = strlen(input);
    if (n == 0) return INVALID_STRING;

    for (i = 0; i < n; i++) {
        if (isdigit(input[i]) || input[i] == '-' || input[i] == '+')
            continue;
        return INVALID_STRING;
    }

    return atoi(input);
}


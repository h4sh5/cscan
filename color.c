#include <stdarg.h>
#include "color.h"

/* print a string in a certain color to stream */
void print_color(FILE *stream, const char *color, char *string, ) {
    fprintf(stream, "%s%s%s\n", color, string, RESET);
}

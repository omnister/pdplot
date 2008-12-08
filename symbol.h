#include <string.h>

typedef struct Symbol {     /* symbol table entry */
    char    *name;
    int     index;
    struct Symbol   *next;  /* to link to another */
} Symbol;

Symbol  *install(), *lookup();
void initsym();

#include "dictionary.h"
#include "statement.h"

// Program
typedef struct program_t{
    BOOL valid;
    stmt* first_statement;
    dictionary functions;
} program;
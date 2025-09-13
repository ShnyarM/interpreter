CC = gcc
CFLAGS = -Wall -O3

OBJS = _parser_helper.o types.o data.o expression.o statement.o dictionary.o function.o variable_access.o strings.o type_parser.o expr_parser.o stmt_parser.o function_parser.o built_in_functions.o executor.o main.o helper.o 

parse: $(OBJS)
	$(CC) $(CFLAGS) -o parse $(OBJS)

parse_dbg: CFLAGS = -Wall -O0 -g
parse_dbg: $(OBJS)
	$(CC) $(CFLAGS) -g -o parse_dbg $(OBJS)

# Object files

## Structs

types.o: structs/types.c structs/types.h helper.h
	$(CC) $(CFLAGS) -c structs/types.c

data.o: structs/data.c structs/data.h structs/types.h structs/function.h structs/strings.h helper.h
	$(CC) $(CFLAGS) -c structs/data.c

expression.o: structs/expression.c structs/expression.h structs/variable_access.h structs/data.h
	$(CC) $(CFLAGS) -c structs/expression.c

statement.o: structs/statement.c structs/statement.h structs/types.h structs/variable_access.h helper.h
	$(CC) $(CFLAGS) -c structs/statement.c

dictionary.o: structs/dictionary.c structs/dictionary.h structs/data.h helper.h
	$(CC) $(CFLAGS) -c structs/dictionary.c

function.o: structs/function.c structs/function.h structs/data.h structs/statement.h
	$(CC) $(CFLAGS) -c structs/function.c

variable_access.o: structs/variable_access.c structs/variable_access.h structs/expression.h helper.h
	$(CC) $(CFLAGS) -c structs/variable_access.c

strings.o: structs/strings.c structs/strings.h structs/data.h structs/types.h helper.h
	$(CC) $(CFLAGS) -c structs/strings.c



## Parsing

_parser_helper.o: parsing/_parser_helper.c parsing/_parser_helper.h structs/expression.h helper.h
	$(CC) $(CFLAGS) -c parsing/_parser_helper.c

type_parser.o: parsing/type_parser.c parsing/type_parser.h parsing/_parser_helper.h structs/types.h helper.h
	$(CC) $(CFLAGS) -c parsing/type_parser.c

expr_parser.o: parsing/expr_parser.c parsing/expr_parser.h parsing/_parser_helper.h structs/expression.h helper.h
	$(CC) $(CFLAGS) -c parsing/expr_parser.c

stmt_parser.o: parsing/stmt_parser.c parsing/stmt_parser.h parsing/expr_parser.h parsing/_parser_helper.h parsing/type_parser.h structs/statement.h structs/expression.h
	$(CC) $(CFLAGS) -c parsing/stmt_parser.c

function_parser.o: parsing/function_parser.c parsing/function_parser.h parsing/stmt_parser.h parsing/expr_parser.h parsing/type_parser.h parsing/_parser_helper.h structs/function.h
	$(CC) $(CFLAGS) -c parsing/function_parser.c



## Executing

built_in_functions.o: executing/built_in_functions.c structs/function.h structs/strings.h structs/data.h
	$(CC) $(CFLAGS) -c executing/built_in_functions.c

executor.o: executing/executor.c executing/executor.h structs/statement.h structs/dictionary.h structs/program.h structs/expression.h structs/data.h structs/function.h helper.h
	$(CC) $(CFLAGS) -c executing/executor.c



## Rest
main.o: main.c helper.h parsing/stmt_parser.h parsing/expr_parser.h parsing/function_parser.h parsing/_parser_helper.h executing/executor.h structs/dictionary.h structs/program.h structs/function.h
	$(CC) $(CFLAGS) -c main.c

helper.o: helper.c helper.h
	$(CC) $(CFLAGS) -c helper.c



# Clean

clean:
	rm -f *~ *.o parse parse_dbg
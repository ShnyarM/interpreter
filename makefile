CC = gcc
CFLAGS = -Wall

OBJS = main.o helper.o dictionary.o data.o executor.o _parser_helper.o expr_parser.o stmt_parser.o function_parser.o

parse: $(OBJS)
	$(CC) $(CFLAGS) -o parse $(OBJS)

parse_dbg: CFLAGS = -Wall -O0 -g
parse_dbg: $(OBJS)
	$(CC) $(CFLAGS) -g -o parse_dbg $(OBJS)

main.o: main.c helper.h parsing/stmt_parser.h parsing/expr_parser.h parsing/function_parser.h dictionary.h general_structs.h
	$(CC) $(CFLAGS) -c main.c

helper.o: helper.c helper.h
	$(CC) $(CFLAGS) -c helper.c

dictionary.o: dictionary.c helper.h dictionary.h data.h
	$(CC) $(CFLAGS) -c dictionary.c

data.o: data.c helper.h general_structs.h data.h
	$(CC) $(CFLAGS) -c data.c

executor.o: executing/executor.c executing/executor.h helper.h general_structs.h dictionary.h data.h
	$(CC) $(CFLAGS) -c executing/executor.c

_parser_helper.o: parsing/_parser_helper.c parsing/_parser_helper.h helper.h general_structs.h
	$(CC) $(CFLAGS) -c parsing/_parser_helper.c

expr_parser.o: parsing/expr_parser.c parsing/expr_parser.h parsing/_parser_helper.h helper.h general_structs.h data.h
	$(CC) $(CFLAGS) -c parsing/expr_parser.c

stmt_parser.o: parsing/stmt_parser.c parsing/stmt_parser.h parsing/expr_parser.h parsing/_parser_helper.h helper.h general_structs.h data.h
	$(CC) $(CFLAGS) -c parsing/stmt_parser.c

function_parser.o: parsing/function_parser.c parsing/function_parser.h parsing/stmt_parser.h parsing/expr_parser.h parsing/_parser_helper.h helper.h general_structs.h data.h
	$(CC) $(CFLAGS) -c parsing/function_parser.c

clean:
	rm -f *~ *.o parse parse_dbg
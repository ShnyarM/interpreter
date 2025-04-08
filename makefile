main: main.c helper.c dictionary.c
	gcc -o parse main.c helper.c dictionary.c

debug: main.c helper.c dictionary.c
	gcc -g -o parse_dbg main.c helper.c dictionary.c
main: main.c helper.c dictionary.c
	gcc -o main main.c helper.c dictionary.c

debug: main.c helper.c dictionary.c
	gcc -g -o main_dbg main.c helper.c dictionary.c
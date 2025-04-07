main: main.c helper.c
	gcc -o main main.c helper.c

debug: main.c helper.c
	gcc -g -o main_dbg main.c helper.c
main_debug: main_debug.c
	gcc -o main_debug main_debug.c -lbluetooth -lncurses -lm

main: main.c
	gcc -o main main.c -lbluetooth -g -lGL -lglut -lGLU -lm

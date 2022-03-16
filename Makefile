all: mysh

mysh: mysh.c
	gcc $^ -o $@

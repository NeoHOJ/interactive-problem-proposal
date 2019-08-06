CC = gcc

all: 23 23m

23: 23.c 23.user.c
23m: 23m.c 23.checker.c

run: 23
	../../nsjail --config nsjail-23.cfg --cwd ${PWD} -- ./23
clean:
	rm 23 23m

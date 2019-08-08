CC = gcc
CFLAGS = -O2

all: 23 23m 23b

23: 23.c 23.user.c
23m: 23m.c 23.checker.c
23b: 23b.c

run: 23
	../../nsjail --config nsjail-23.cfg --cwd ${PWD} -- ./23
bench: 23b
	../../nsjail --chroot / --cwd ${PWD} ./23b
clean:
	rm 23 23m 23b

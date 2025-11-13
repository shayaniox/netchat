CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -g -I.
LDFLAGS :=
LDLIBS  :=

srcfiles = $(wildcard *.c)
objfiles = $(patsubst %.c,%.o,$(srcfiles))

run: main
	@./main

main: $(objfiles)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main *.o

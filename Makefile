cc=gcc
flags= -std=c23 -Wall -Wextra -Werror -g -fsanitize=address -fsanitize=bounds
libc=$(wildcard **/*.c)
libh=$(patsubst %.c,%.h, $(libc))
libobj=$(patsubst %.c,%.o, $(libc))
bin=main

.PHONY: all
all: $(bin)

$(bin): main.o $(libobj)
	$(cc) $(flags) -o $@ $^
	./$(bin)

%.o: %.c $(libh)
	$(cc) $(flags) -c $< -o $@

clean:
	rm -f $(bin) $(lib)

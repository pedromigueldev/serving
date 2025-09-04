cc2=gcc
cc=~/Downloads/filc-0.670-linux-x86_64/build/bin/clang
flags= -std=c23 -Wall -Wextra -Werror -g #-fsanitize=address -fsanitize=bounds
libc=$(wildcard **/*.c)
libh=$(patsubst %.c,%.h, $(libc))
libobj=$(patsubst %.c,%.o, $(libc))
bin=main

.PHONY: all
all: $(bin)

exec:
	./$(bin)

$(bin): main.o $(libobj)
	$(cc) $(flags) -o $@ $^

%.o: %.c $(libh)
	$(cc) $(flags) -c $< -o $@

clean:
	rm -f $(bin) $(lib)

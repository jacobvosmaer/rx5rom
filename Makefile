CFLAGS = -Wall -pedantic -std=gnu89 -fno-common
EXE = rx5-ls rx5-split
all: $(EXE)
dev: CFLAGS += -Werror -g
dev: all
rx5-ls: rx5.o
rx5-split: wav.o rx5.o
clean:
	rm -f -- $(EXE)

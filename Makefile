CFLAGS = -Wall -pedantic -std=gnu89 -fno-common
EXE = rx5-ls rx5-split
all: $(EXE)
dev: CFLAGS += -Werror
dev: all
rx5-split: wav.o
clean:
	rm -f -- $(EXE)

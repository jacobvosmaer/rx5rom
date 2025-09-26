CFLAGS = -Wall -pedantic -std=gnu89 -fno-common
EXE = rx5-ls
all: $(EXE)
dev: CFLAGS += -Werror
dev: all
clean:
	rm -f -- $(EXE)

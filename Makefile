CFLAGS = -Wall -pedantic -std=gnu89 -fno-common
EXE = rx5-ls rx5-split rx5-program
OBJ = rx5.o wav.o
ifeq ($(shell uname), Linux)
HIDAPI ?= hidapi-hidraw
else
HIDAPI ?= hidapi
endif
all: $(EXE)
dev: CFLAGS += -Werror -g
dev: all
rx5-ls: rx5.o
rx5-split: wav.o rx5.o
rx5-program: CFLAGS += $(shell pkg-config --cflags $(HIDAPI))
rx5-program: LDLIBS += $(shell pkg-config --libs $(HIDAPI))
clean:
	rm -rf -- $(EXE) $(OBJ)

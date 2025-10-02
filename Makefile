CFLAGS = -Wall -pedantic -std=gnu89 -fno-common
EXE = rx5-ls rx5-split rx5-program
OBJ = rx5.o wav.o rawhid/hid_LINUX.o
all: $(EXE)
dev: CFLAGS += -Werror -g
dev: all
rx5-ls: rx5.o
rx5-split: wav.o rx5.o
rx5-program: CPPFLAGS += -DOS_LINUX -Irawhid
rx5-program: CFLAGS += $(shell pkg-config --cflags libusb) -std=c99
rx5-program: LDFLAGS += $(shell pkg-config --libs libusb)
rx5-program: rawhid/hid_LINUX.o

clean:
	rm -rf -- $(EXE) $(OBJ)

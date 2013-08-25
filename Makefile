CC = cc
CFLAGS = -std=gnu11 -O3 -I.

PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin

LIBS=-lm
ARCH=$(uname -m)

## TODO: must write the main LSD daemon
# DEPS =
# OBJ = battery.o clock.o cpu.o memory.o thermal.o wifi.o
BINS = battery clock cpu memory disk thermal wifi

all: $(BINS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

cpu: cpu.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

battery: battery.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clock: clock.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

memory: memory.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

disk: disk.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

thermal: thermal.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

wifi: wifi.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

install:
	mkdir -p "$(DESTDIR)$(BINPREFIX)"
	cp -p $(BINS) "$(DESTDIR)$(BINPREFIX)"

uninstall:
	rm -f "$(DESTDIR)$(BINPREFIX)"/{$(BINS)}

.PHONY: clean

clean:
	rm -f *.o $(BINS)

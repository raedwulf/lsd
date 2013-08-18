CC = cc
CFLAGS = -std=gnu11 -O3 -I.

LIBS=-lm
ARCH=$(uname -m)

## TODO: must write the main LSD daemon
# DEPS =
# OBJ = battery.o clock.o cpu.o memory.o thermal.o wifi.o

all: battery clock memory thermal wifi

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

battery: battery.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clock: clock.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

memory: memory.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

thermal: thermal.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

wifi: wifi.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o

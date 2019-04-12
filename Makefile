CC=gcc
CFLAGS=-Wall -g -pedantic -std=gnu99 -fdiagnostics-color

DEFAULT=all

all: cscan

cscan: cscan.c Makefile extraUtils.h extraUtils.c
	$(CC) $(CFLAGS) -c -g cscan.c -o cscan.o -pthread
	$(CC) $(CFLAGS) -g -c extraUtils.c -o extraUtils.o
	$(CC) $(CFLAGS) -pthread -g extraUtils.o cscan.o -o cscan -pthread

debug: cscan.c Makefile extraUtils.h extraUtils.c
	$(CC) $(CFLAGS) -DDEBUG -c -g cscan.c -o cscan.o -pthread
	$(CC) $(CFLAGS) -DDEBUG -g -c extraUtils.c -o extraUtils.o
	$(CC) $(CFLAGS) -pthread -g extraUtils.o cscan.o -o cscan -pthread

clean:
	rm cscan

# Makefile RC 2021/2022 @ IST

CC = gcc
CFLAGS = -g -o -Wall

.PHONY = all clean run

all: DS

DS: udpandtcp.o auxfunctions.o main.o
	$(CC) $(CFLAGS) -o DS udpandtcp.o auxfunctions.o main.o

main.o: main.c auxfunctions.h
	$(CC) $(CFLAGS) -o main.o -c main.c

auxfunctions.o: auxfunctions.c auxfunctions.h udpandtcp.h
	$(CC) $(CFLAGS) -o auxfunctions.o -c auxfunctions.c

udpandtcp.o: udpandtcp.c udpandtcp.h
	$(CC) $(CFLAGS) -o udpandtcp.o -c udpandtcp.c

clean:
	@echo Cleaning...
	rm -f *.o DS 

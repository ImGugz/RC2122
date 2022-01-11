# Makefile RC 2021/2022 (Grupo 18) @ IST

CC = gcc
CFLAGS = -g -o -Wall

SUBDIRS = . client server
SUBDIRSCLEAN=$(addsuffix clean,$(SUBDIRS))

.PHONY = all clean run

all: user

user: udpandtcp.o auxfunctions.o execute-commands.o commands.o main.o
	$(CC) $(CFLAGS) -o user udpandtcp.o auxfunctions.o execute-commands.o commands.o main.o

main.o: main.c execute-commands.h auxfunctions.h
	$(CC) $(CFLAGS) -o main.o -c main.c

commands.o: commands.c commands.h execute-commands.h auxfunctions.h
	$(CC) $(CFLAGS) -o commands.o -c commands.c

execute-commands.o: execute-commands.c execute-commands.h udpandtcp.h auxfunctions.h
	$(CC) $(CFLAGS) -o execute-commands.o -c execute-commands.c

auxfunctions.o: auxfunctions.c auxfunctions.h udpandtcp.h
	$(CC) $(CFLAGS) -o auxfunctions.o -c auxfunctions.c

udpandtcp.o: udpandtcp.c udpandtcp.h execute-commands.h
	$(CC) $(CFLAGS) -o udpandtcp.o -c udpandtcp.c

clean: $(SUBDIRSCLEAN)

	@echo Cleaning...
	rm *.o $(objects)

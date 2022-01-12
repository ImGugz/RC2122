# Makefile RC 2021/2022 (Grupo 18) @ IST
CC=gcc
CFLAGS= -Wall -g

ODIR=obj

DEPS = centralizedmsg-api.h centralizedmsg-api-constants.h client/centralizedmsg-client-api.h 

_OBJ = centralizedmsg-api.o centralizedmsg-client.o centralizedmsg-client-api.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

.PHONY: all clean run

all: user

user: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

$(ODIR)/%.o: %.c $(DEPS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $< 

obj/centralizedmsg-client.o: client/centralizedmsg-client.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

obj/centralizedmsg-client-api.o: client/centralizedmsg-client-api.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 


clean:
	@echo Cleaning...
	rm -rf $(ODIR)
	rm -f *~ core $(INCDIR)/*~ user

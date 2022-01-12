#CC = gcc
#CFLAGS = -g -Wall
#
#ODIR = obj
#
#DEPS = centralizedmsg-api.h centralizedmsg-api-constants.h client/centralizedmsg-client-api.h
#
#_OBJ = centralizedmsg-api.o centralizedmsg-client.o centralizedmsg-client-api.o
#OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
#
#.PHONY: all clean run
#
#
#$(ODIR)/%.o: %.c $(DEPS)
#	$(CC) $(CFLAGS) -c -o $@ $< 
#
#user: $(OBJ)
#	$(CC) -o $@ $^ $(CFLAGS)
#
#
#obj/centralizedmsg-api.o: centralizedmsg-api.c centralizedmsg-api.h centralizedmsg-api-constants.h
#	$(CC) $(CFLAGS) -o centralizedmsg-api.o -c centralizedmsg-api.c
#
#obj/centralizedmsg-client.o: client/centralizedmsg-client.c
#	$(CC) $(CFLAGS) -o centralizedmsg-client.o -c client/centralizedmsg-client.c
#
#obj/centralizedmsg-client-api.o: client/centralizedmsg-client-api.c client/centralizedmsg-client-api.h
#	$(CC) $(CFLAGS) -o centralizedmsg-client-api.o -c client/centralizedmsg-client-api.c
#
#clean:
#	@echo Cleaning...
#	rm -f client/*.o *.o user

CC=gcc
CFLAGS= -Wall -g

ODIR=obj

DEPS = centralizedmsg-api.h centralizedmsg-api-constants.h client/centralizedmsg-client-api.h 

_OBJ = centralizedmsg-api.o centralizedmsg-client.o centralizedmsg-client-api.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


all: user

user: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

obj/centralizedmsg-client.o: client/centralizedmsg-client.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

obj/centralizedmsg-client-api.o: client/centralizedmsg-client-api.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
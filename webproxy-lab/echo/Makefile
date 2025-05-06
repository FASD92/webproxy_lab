# Makefile for CS:APP echo client/server

CC = gcc
CFLAGS = -Wall -O2
OBJS = csapp.o echo.o

all: echoclient echoserveri

echoclient: echoclient.o $(OBJS)
	$(CC) $(CFLAGS) -o echoclient echoclient.o $(OBJS)

echoserveri: echoserveri.o $(OBJS)
	$(CC) $(CFLAGS) -o echoserveri echoserveri.o $(OBJS)

echoclient.o: echoclient.c csapp.h
	$(CC) $(CFLAGS) -c echoclient.c

echoserveri.o: echoserveri.c csapp.h
	$(CC) $(CFLAGS) -c echoserveri.c

echo.o: echo.c csapp.h
	$(CC) $(CFLAGS) -c echo.c

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

clean:
	rm -f *.o echoclient echoserveri
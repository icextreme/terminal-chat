CC_C = gcc

CFLAGS = -Werror -Wall -g -std=c99 -D _GNU_SOURCE -pthread

default: all

all: t-chat

t-chat: t-chat.o network.o list.o ui.o
	$(CC_C) $(CFLAGS) -o t-chat t-chat.o network.o list.o ui.o

t-chat.o: t-chat.c 
	$(CC_C) $(CFLAGS) -c t-chat.c
	
network.o: network.c network.h list.h
	$(CC_C) $(CFLAGS) -c network.c

list.o: list.c list.h
	$(CC_C) $(CFLAGS) -c list.c

ui.o: ui.c ui.h list.h
	$(CC_C) $(CFLAGS) -c ui.c

clean:
	rm -f *o t-chat
	rm -f *o list
	rm -f *o network
	rm -f *o ui
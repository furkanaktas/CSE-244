all: final

final: 
	gcc -c -std=c99 -D_BSD_SOURCE -D_XOPEN_SOURCE clients.c && gcc -pthread -o clients clients.o -lm && gcc -c -std=c99 -D_BSD_SOURCE -D_XOPEN_SOURCE server.c && gcc -pthread -o server server.o -lm

clean:
	rm -f *.o
	rm server
	rm clients

all: midterm

midterm: 
	gcc -c seeWhat.c && gcc seeWhat.c -o seeWhat -lm && gcc -c timeServer.c && gcc timeServer.c -o timeServer -lm && gcc -c showResult.c && gcc showResult.c -o showResult

clean:
	rm -f *.o
	rm  timeServer
	rm seeWhat 
	rm showResult


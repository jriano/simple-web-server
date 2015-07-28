all: ws

ws: ws.o headers.h
	gcc -g -O0 -Wall ws.o -o ws -static -lm

ws.o: ws.c headers.h
	gcc -g -c -Wall ws.c

clean:
	rm -rf *.o ws core *~


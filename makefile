parametros=-Wall
nome=vina++

all: $(nome)

$(nome): main.o libvpp.o
	gcc main.o libvpp.o $(parametros) -o $(nome)

main.o: main.c
	gcc -c main.c $(parametros)

libvpp.o: libvpp.c libvpp.h
	gcc -c libvpp.c $(parametros)

clean:
	rm -f *.gch *.o $(nome)
CC := gcc
OBJ := main.o services.o

.PHONY: clean all

all: main 

main: $(OBJ)
	$(CC) $^ -o $@ 

main.o: main.c
	$(CC) -c main.c

services.o: services.c
	$(CC) -c services.c

clean:
	rm -f *.o main

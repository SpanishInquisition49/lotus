SHELL	:=	/bin/bash
CC		:=	gcc
CFLAGS 	:=	-g -Wall -pedantic -Wextra

objects	:= 	./src/main.o ./lib/list.o ./lib/scanner.o ./lib/token.o ./lib/errors.o
executable:= lambda

.PHONY	:=	clean


$(executable): $(objects)
	$(CC) $(CFLAGS) $^ -o $@

./src/main.o: ./src/main.c
./lib/scanner.o: ./lib/scanner.c ./lib/scanner.h ./lib/token.h ./lib/list.h ./lib/errors.h ./lib/keyworks.h
./lib/list.o: ./lib/list.c ./lib/list.h
./lib/token.o: ./lib/token.c ./lib/token.h ./lib/list.h ./lib/errors.h
./lib/errors.o: ./lib/errors.c ./lib/errors.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-@rm -f src/*.o lib/*.o $(executable)

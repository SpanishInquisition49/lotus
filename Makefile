SHELL		:=	/bin/bash
CC			:=	gcc
CFLAGS		:=	-g -Wall -pedantic -Wextra

# Targets
main_c		:= ./src/main.c
main_o		:= ./src/main.o

keywords_h	:= ./lib/keywords.h

syntax_h	:= ./lib/syntax.h
syntax_c	:= ./lib/syntax.c
syntax_o	:= ./lib/syntax.o

token_h		:= ./lib/token.h
token_c		:= ./lib/token.c
token_o		:= ./lib/token.o

scanner_h	:= ./lib/scanner.h
scanner_c	:= ./lib/scanner.c
scanner_o	:= ./lib/scanner.o

parser_h	:= ./lib/parser.h
parser_c	:= ./lib/parser.c
parser_o	:= ./lib/parser.o

list_h		:= ./lib/list.h
list_c		:= ./lib/list.c
list_o		:= ./lib/list.o

memory_h	:= ./lib/memory.h
memory_c	:= ./lib/memory.c
memory_o	:= ./lib/memory.o

errors_h	:= ./lib/errors.h
errors_c	:= ./lib/errors.c
errors_o	:= ./lib/errors.o

objects		:= 	$(main_o) $(list_o) $(scanner_o) $(parser_o) $(token_o) $(syntax_o) $(errors_o) $(memory_o)
executable	:= lambda

.PHONY		:=	clean valgring


$(executable): $(objects)
	$(CC) $(CFLAGS) $^ -o $@

$(main_o): $(main_c)
$(list_o): $(list_c) $(list_h)
$(errors_o): $(errors_c) $(errors_h)
$(memory_o): $(memory_c) $(memory_h)
$(token_o): $(token_c) $(token_h) $(list_h) $(errors_h) $(memory_h)
$(syntax_o): $(syntax_c) $(syntax_h) $(token_h) $(list_h) $(memory_h)
$(scanner_o): $(scanner_c) $(scanner_h) $(token_h) $(list_h) $(errors_h) $(memory_h) $(keywords_h)
$(parser_o): $(parser_c) $(parser_h) $(token_h) $(list_h) $(errors_h) $(syntax_h) $(syntax_h)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-@rm -f src/*.o lib/*.o $(executable)

valgrind: $(executable)
	-@valgrind --leak-check=full --track-origins=yes --log-file=lambda.log ./lambda example

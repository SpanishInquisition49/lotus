SHELL		:=	/bin/bash
CC			:=	gcc
CFLAGS		:=	-g -Wall -pedantic -Wextra -Wunknown-pragmas
VFLAGS		:= 	--leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes

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

interpreter_h := ./lib/interpreter.h
interpreter_c := ./lib/interpreter.c
interpreter_o := ./lib/interpreter.o

list_h		:= ./lib/list.h
list_c		:= ./lib/list.c
list_o		:= ./lib/list.o

thread_h 	:= ./lib/thread.h
thread_c 	:= ./lib/thread.c
thread_o 	:= ./lib/thread.o

memory_h	:= ./lib/memory.h
memory_c	:= ./lib/memory.c
memory_o	:= ./lib/memory.o

errors_h	:= ./lib/errors.h
errors_c	:= ./lib/errors.c
errors_o	:= ./lib/errors.o

environment_h := ./lib/environment.h
environment_c := ./lib/environment.c
environment_o := ./lib/environment.o

garbage_h 	:= ./lib/garbage.h
garbage_c 	:= ./lib/garbage.c
garbage_o 	:= ./lib/garbage.o

config_h	:= ./lib/config.h
config_c	:= ./lib/config.c
config_o	:= ./lib/config.o

objects		:= 	$(main_o) $(list_o) $(scanner_o) $(parser_o) $(interpreter_o) $(token_o) $(syntax_o) $(errors_o) $(memory_o) $(config_o) $(environment_o) $(garbage_o) $(thread_o)
executable	:= lotus

.PHONY		:=	clean valgring clean_logs check


$(executable): $(objects)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lm

$(main_o): $(main_c)
$(list_o): $(list_c) $(list_h)
$(config_o): $(config_c) $(config_h)
$(errors_o): $(errors_c) $(errors_h)
$(memory_o): $(memory_c) $(memory_h)
$(thread_o): $(thread_c) $(thread_h)
$(environment_o): $(environment_c) $(environment_h) $(memory_h) $(list_h)
$(garbage_o): $(garbage_c) $(garbage_h) $(memory_h) $(environment_h) $(list_h) $(thread_h) $(syntax_h)
$(token_o): $(token_c) $(token_h) $(list_h) $(errors_h) $(memory_h)
$(syntax_o): $(syntax_c) $(syntax_h) $(token_h) $(list_h) $(memory_h)
$(scanner_o): $(scanner_c) $(scanner_h) $(token_h) $(list_h) $(errors_h) $(memory_h) $(keywords_h)
$(parser_o): $(parser_c) $(parser_h) $(token_h) $(list_h) $(errors_h) $(syntax_h) $(syntax_h)
$(interpreter_o): $(interpreter_c) $(interpreter_h) $(syntax_h) $(memory_h) $(errors_h) $(environment_h) $(garbage_h)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-@rm -f src/*.o lib/*.o $(executable)

valgrind: $(executable)
	-@valgrind $(VFLAGS) --log-file=./logs/lambda-arithmetic-%p-%n.log ./$(executable) ./foo

clean_logs:
	-@rm -f ./logs/*

check: $(executable)
	-@./tester

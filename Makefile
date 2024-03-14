SHELL						:=	/bin/bash
CC							:=	gcc
CFLAGS					:=	-g -Wall -pedantic -Wextra -Wno-unknown-pragmas -Wno-unused-parameter
VFLAGS					:= 	--leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes

# Targets
executable			:= lotus
main_o					:= ./src/main.o
syntax_o				:= ./lib/syntax.o
token_o					:= ./lib/token.o
scanner_o				:= ./lib/scanner.o
parser_o				:= ./lib/parser.o
interpreter_o 	:= ./lib/interpreter.o
list_o					:= ./lib/list.o
thread_o 				:= ./lib/thread.o
memory_o				:= ./lib/memory.o
errors_o				:= ./lib/errors.o
environment_o 	:= ./lib/environment.o
garbage_o 			:= ./lib/garbage.o
config_o				:= ./lib/config.o

objects					:= 	$(main_o) \
										$(list_o) \
										$(scanner_o) \
										$(parser_o) \
										$(interpreter_o) \
										$(token_o) \
										$(syntax_o) \
										$(errors_o) \
										$(memory_o) \
										$(config_o) \
										$(environment_o) \
										$(garbage_o) \
										$(thread_o)

.PHONY					:=	clean valgring clean_logs check


$(executable): $(objects)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lm


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-@rm -f src/*.o lib/*.o $(executable)

valgrind: $(executable)
	-@valgrind $(VFLAGS) --log-file=./logs/lambda-arithmetic-%p-%n.log ./$(executable) ./test/functions.lts

clean_logs:
	-@rm -f ./logs/*

check: $(executable)
	-@./tester

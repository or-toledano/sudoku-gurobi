CC = gcc
OBJS = main.o main_aux.o parser.o game.o board.o files.o list.o optimized_solver.o solution_counter.o stack.o
EXEC = sudoku-console
COMP_FLAGS = -ansi -O3 -Wall -Wextra -Werror -pedantic-errors
GUROBI_COMP = -I/usr/local/lib/gurobi563/include
GUROBI_LIB = -L/usr/local/lib/gurobi563/lib -lgurobi56

all: $(OBJS)
	$(CC) $(OBJS) $(GUROBI_LIB) -o $(EXEC) -lm
main.o: main.c main_aux.h
	$(CC) $(COMP_FLAG) -c $*.c
main_aux.o: main_aux.c main_aux.h game.h parser.h
	$(CC) $(COMP_FLAG) -c $*.c
parser.o: parser.c parser.h
	$(CC) $(COMP_FLAG) -c $*.c
game.o: game.c game.h board.h main_aux.h files.h solution_counter.h optimized_solver.h list.h
	$(CC) $(COMP_FLAG) -c $*.c
board.o: board.c board.h main_aux.h
	$(CC) $(COMP_FLAG) -c $*.c
files.o: files.c files.h game.h board.h
	$(CC) $(COMP_FLAG) -c $*.c
list.o: list.c list.h main_aux.h board.h
	$(CC) $(COMP_FLAG) -c $*.c
optimized_solver.o: optimized_solver.c optimized_solver.h main_aux.h board.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
solution_counter.o: solution_counter.c solution_counter.h board.h stack.h main_aux.h 
	$(CC) $(COMP_FLAG) -c $*.c
stack.o: stack.c stack.h main_aux.h
	$(CC) $(COMP_FLAG) -c $*.c
clean:
	rm -f $(OBJS) $(EXEC)

CFLAGS = -ansi -pedantic -std=c99 -Wextra -Wall
HFILES = string.h stack.h syntaxtree.h generator.h
OFILES = main.o string.o stack.o syntaxtree.o generator.o
CC = gcc

main: $(OFILES)
	$(CC) $(CFLAGS) -o $@ $(OFILES) && rm $(OFILES)

mprint: main print

print:
	./main > out

main.o: main.c $(HFILES)
	$(CC) $(CFLAGS) -c main.c
	
string.o: string.c $(HFILES)
	$(CC) $(CFLAGS) -c string.c

stack.o: stack.c $(HFILES)
	$(CC) $(CFLAGS) -c stack.c

generator.o: generator.c $(HFILES)
	$(CC) $(CFLAGS) -c generator.c

syntaxtree.o: syntaxtree.c $(HFILES)
	$(CC) $(CFLAGS) -c syntaxtree.c

clean:
	rm $(OFILES)
CFLAGS = -ansi -pedantic -std=c99 -Wall -Wextra
HFILES = string.h scanner.h generator.h syntaxtree.h stack.h postfix.h parser.h symtable.h precedence.h
OFILES = scanner.o string.o generator.o syntaxtree.o stack.o parser.o precedence.o postfix.o main.o symtable.o
CC = gcc

main: $(OFILES)
	$(CC) $(CFLAGS) -o main $(OFILES)
	
main.o: main.c $(HFILES)
	$(CC) $(CFLAGS) -c main.c
	
scanner.o: scanner.c $(HFILES)
	$(CC) $(CFLAGS) -c scanner.c
	
string.o: string.c $(HFILES)
	$(CC) $(CFLAGS) -c string.c

generator.o: generator.c $(HFILES)
	$(CC) $(CFLAGS) -c generator.c
	
syntaxtree.o: syntaxtree.c $(HFILES)
	$(CC) $(CFLAGS) -c syntaxtree.c
	
postfix.o: postfix.c $(HFILES)
	$(CC) $(CFLAGS) -c postfix.c

stack.o: stack.c $(HFILES)
	$(CC) $(CFLAGS) -c stack.c

parser.o: parser.c $(HFILES)
	$(CC) $(CFLAGS) -c parser.c

symtable.o: symtable.c $(HFILES)
	$(CC) $(CFLAGS) -c symtable.c


precedence.o: precedence.c $(HFILES)
	$(CC) $(CFLAGS) -c precedence.c
	
clean:
	rm $(OFILES)
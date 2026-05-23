CC = gcc
CFLAGS = -Wall -Wextra -std=c99
OBJS = main.o lex.o parser.o symtab.o diag.o opt.o log.o gerador.o
EXEC = salc

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

main.o: main.c lex.h parser.h symtab.h diag.h opt.h log.h
	$(CC) $(CFLAGS) -c main.c

lex.o: lex.c lex.h diag.h log.h
	$(CC) $(CFLAGS) -c lex.c

parser.o: parser.c parser.h lex.h symtab.h diag.h
	$(CC) $(CFLAGS) -c parser.c

symtab.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c symtab.c

diag.o: diag.c diag.h
	$(CC) $(CFLAGS) -c diag.c

opt.o: opt.c opt.h
	$(CC) $(CFLAGS) -c opt.c

log.o: log.c log.h lex.h symtab.h
	$(CC) $(CFLAGS) -c log.c

gerador.o: gerador.c gerador.h
	$(CC) $(CFLAGS) -c gerador.c
	
clean:
	rm -f *.o $(EXEC) *.tk *.ts *.trc
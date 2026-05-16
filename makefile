CC = gcc
CFLAGS = -Wall -Wextra -std=c99

TARGET = salc

SOURCES = main.c lex.c parser.c symtab.c diag.c opt.c log.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) salc.exe
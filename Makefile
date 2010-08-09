OBJS=main.o type.o var.o expr.o func.o stmt.o \
	parser.o unit.o op.o grammar.y.o grammar.l.o
CFLAGS=-std=c99 -c -g -I. -Wall -Werror -pedantic


apc: $(OBJS)
	$(CC) -g -o $@ $^ 

clean:
	rm -rf *.o mc grammar.y.c grammar.l.c


grammar.y.c: grammar.y
	bison --defines=grammar.h --output=$@ $^

grammar.l.c: grammar.l
	flex --outfile=$@ $^


%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^ 

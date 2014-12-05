EXEC = p2pchatd

all: chatd.c
	gcc chatd.c -o $(EXEC)
	
debug:
	gcc chatd.c -o $(EXEC) -DDEBUG -g
	
clean:
	rm -f $(EXEC) *~

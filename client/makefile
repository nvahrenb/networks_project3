EXEC = p2pchat

all: chat.c
	gcc chat.c -o $(EXEC)
	
debug:
	gcc chat.c -o $(EXEC) -DDEBUG -g
	
clean:
	rm -f $(EXEC) *~

EXEC = p2pchat

all: chat.c
	gcc chat.c -o $(EXEC) -lpthread
	
debug:
	gcc chat.c -o $(EXEC) -DDEBUG -g -lpthread
	
clean:
	rm -f $(EXEC) *~

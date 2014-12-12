EXEC = p2pchat

all: chat.c
	gcc chat.c -o $(EXEC) -lSDL -lSDL_mixer
	
debug:
	gcc chat.c -o $(EXEC) -DDEBUG -g -lSDL -lSDL_mixer
	
clean:
	rm -f $(EXEC) *~

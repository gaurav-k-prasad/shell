TARGET = gshell
OBJ = src/main.c src/parser.c src/helpers.c src/builtins.c src/executor.c src/input.c src/forgettingDLL.c src/aiParser.c
CC = gcc

all: 
	$(CC) -g -o $(TARGET) $(OBJ)
clean:
	rm -f *.o
fclean: clean
	rm -f $(TARGET)
re: fclean all

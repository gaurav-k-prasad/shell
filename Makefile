TARGET = myshell
OBJ = src/main.c src/parser.c src/helpers.c src/builtins.c src/executor.c
CC = gcc

all: 
	$(CC) -o $(TARGET) $(OBJ)
clean:
	rm -f *.o
fclean: clean
	rm -f $(TARGET)
re: fclean all

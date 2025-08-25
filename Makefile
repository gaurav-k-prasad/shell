TARGET = myshell
OBJ = main.c parser.c helpers.c builtins.c executor.c
CC = gcc

all: 
	$(CC) -o $(TARGET) $(OBJ)
clean:
	rm -f *.o
fclean: clean
	rm -f $(TARGET)
re: fclean all

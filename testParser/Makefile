TARGET = myshell
OBJ = declare.c lex.c main.c
CC = gcc

all: 
	$(CC) -g -o $(TARGET) $(OBJ)
clean:
	rm -f *.o
fclean: clean
	rm -f $(TARGET)
re: fclean all

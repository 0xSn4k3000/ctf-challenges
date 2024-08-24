CC = gcc

TARGET = khp_server
SRC = khp_server.c
OBJ = $(SRC:.c=.o)
HEADER = khp_func.h

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

$(OBJ): $(SRC) $(HEADER)
	$(CC) -c $(SRC)

clean:
	rm -f $(OBJ) $(TARGET)

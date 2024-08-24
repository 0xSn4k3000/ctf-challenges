CC = gcc
CFLAGS = -m32 -lseccomp -I./include -Wl,-z,relro,-z,now -fstack-protector-strong 
TARGET = rm-1337
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

c:
	rm -f $(TARGET)

.PHONY: all clean
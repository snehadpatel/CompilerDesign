CC = gcc
CFLAGS = -Wall -Wextra -g -I./src
SRCS = src/main.c src/lexer.c src/parser.c src/codegen.c
OBJS = $(SRCS:.c=.o)
TARGET = py2c

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

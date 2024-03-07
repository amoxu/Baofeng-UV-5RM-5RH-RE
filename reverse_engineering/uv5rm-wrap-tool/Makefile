CC = gcc
CFLAGS =-g -Wall -Wextra
TARGETS = uv5rm-wrap-tool

all: $(TARGETS)

uv5rm-wrap-tool: encrypt.c main.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)

all: $(TARGETS)

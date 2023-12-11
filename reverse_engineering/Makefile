CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGETS = encrypt decrypt

all: $(TARGETS)

encrypt: encrypt.c
	$(CC) $(CFLAGS) -o encrypt encrypt.c

decrypt: decrypt.c
	$(CC) $(CFLAGS) -o decrypt decrypt.c

clean:
	rm -f $(TARGETS)
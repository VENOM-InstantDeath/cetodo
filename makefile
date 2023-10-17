CC=gcc
CFLAGS=-fsanitize=address -I/usr/include/json-c
LIBS=-lncurses -ljson-c -lasan -ljson-c
DEPS=main.c gears.c libncread/ncread.c libncread/vector.c logger/logger.c linked/linked.c

etodo: $(DEPS)
	$(CC) $(DEPS) $(CFLAGS) $(LIBS) -o etodo -g

CC=gcc
LD = gcc

SRCS = src/http_request.c \
	src/http_response.c \
	src/mime_types.c \
	src/mime.c \
	src/path.c \
	src/error_handler.c \
	src/httpd.c \
	src/deps/strmap/strmap.c

CFLAGS = -Iinclude -Wall -march=native -O3 -fstrict-aliasing #-g -fsanitize=address -fno-omit-frame-pointer
LDFLAGS = -O3 #-g -fsanitize=address

OBJS=$(subst .c,.o,$(SRCS))

all: build

build: $(OBJS)
	$(LD) $(LDFLAGS) -o httpd $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

depend: .depend

.depend: $(SRCS) $(TEST_SRCS)
	$(RM) -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend

clean:
	$(RM) $(OBJS)
	$(RM) httpd

include .depend

CC := gcc
CFLAGS := -g -Wall -I../include
LIBS := -lpthread -lcrypt

SRCS_SERVER := server.c threadpool.c ser_main.c config.c hashtable.c
OBJS_SERVER := $(SRCS_SERVER:.c=.o)
TARGET_SERVER := server

.PHONY: all clean

all: $(TARGET_SERVER)

$(TARGET_SERVER): $(OBJS_SERVER)
<Tab>$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
<Tab>$(CC) $(CFLAGS) -c $< -o $@

clean:
<Tab>rm -f *.o $(TARGET_SERVER)


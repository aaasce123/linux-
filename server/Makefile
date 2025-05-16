# 编译器与编译选项
CC := gcc
CFLAGS := -g -Wall -I../include
LIBS := -lpthread -lcrypt

# 源文件与目标文件
SRCS := server.c threadpool.c ser_main.c config.c hashtable.c user.c
OBJS := $(SRCS:.c=.o)

# 目标文件名
TARGET := server

# 默认目标
.PHONY: all clean

all: $(TARGET)

# 链接目标文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# 编译每个 .c 为 .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理构建文件
clean:
	rm -f *.o $(TARGET)


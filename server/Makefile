# 编译器
CC := gcc

# 编译选项，包含调试和警告，及头文件路径
CFLAGS := -g -Wall -Wno-deprecated-declarations -I../include -I/usr/include/mysql

# 链接库，包含 MySQL 和其他依赖
LIBS := -lmysqlclient -lpthread -lcrypt -lcrypto

# 源文件列表
SRCS := config.c hashtable.c log.c ser_main.c server.c sql.c threadpool.c user.c session.c	sha1.c

# 目标文件列表
OBJS := $(SRCS:.c=.o)

# 目标可执行文件名
TARGET := server

.PHONY: all clean

all: $(TARGET)

# 链接目标文件生成可执行程序
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# 编译规则：.c 编译成 .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)


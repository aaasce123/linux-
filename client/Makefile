# 编译器
CC := gcc

# 编译选项：调试信息，开启所有警告，包含上一级目录的 include 文件
CFLAGS := -g -Wall -I../include -DOPENSSL_API_COMPAT=0x10100000

# 链接需要的库：线程库、加密库、SSL 库和加密库
LIBS := -lpthread -lcrypt -lssl -lcrypto

# 源文件列表
SRCS := client.c cli_main.c

# 目标文件列表，由源文件替换扩展名 .c 为 .o
OBJS := $(SRCS:.c=.o)

# 最终生成的可执行文件名
TARGET := client

# 声明伪目标，防止与实际文件冲突
.PHONY: all clean

# 默认目标，先执行 clean 再编译目标程序
all: clean $(TARGET)

# 链接目标程序，依赖所有目标文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# 编译所有 .c 文件为 .o 文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理所有生成的目标文件和可执行文件
clean:
	rm -f *.o $(TARGET)


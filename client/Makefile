# 编译器和标志
CC := gcc
CFLAGS := -g -Wall -I../include  # 包含上一级目录的 include 文件
LIBS := -lpthread -lcrypt        # 需要的库链接

# 客户端源文件、目标文件和最终生成的可执行文件
SRCS := client.c cli_main.c
OBJS := $(SRCS:.c=.o)
TARGET := client

# .PHONY 声明，避免与实际文件名冲突
.PHONY: all clean

# 默认目标：先清理然后编译客户端
all: clean $(TARGET)

# 编译客户端可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# 编译 .c 文件为 .o 文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理目标文件和可执行文件
clean:
	rm -f *.o $(TARGET)


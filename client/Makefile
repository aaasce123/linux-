# 编译器
CC := gcc

# 编译选项，包含调试、警告、和头文件路径
CFLAGS := -g -Wall -I../include -DOPENSSL_API_COMPAT=0x10100000

# 库搜索路径
LDFLAGS := -L../lib

# 需要链接的库，加入 JWT 静态库，和你原本的库
LIBS := -ll8w8jwt -lmbedcrypto -lmbedtls -lmbedx509 -lpthread -lcrypt -lssl -lcrypto

# 源文件列表
SRCS := client.c cli_main.c

# 目标文件列表
OBJS := $(SRCS:.c=.o)

# 生成的可执行文件
TARGET := client

.PHONY: all clean

all: clean $(TARGET)

# 链接规则，添加 LDFLAGS
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)

# 编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)


# 编译器
CC := gcc

# 编译选项，包含调试和警告，及头文件路径（保持原有）
CFLAGS := -g -Wall -Wno-deprecated-declarations -I../include -I../include/l8w8jwt -I/usr/include/mysql

# 链接库目录，增加 ../lib，和可能的系统库目录（如果你安装在标准位置可以省略）
LDFLAGS := -L../lib

# 链接库，包含 MySQL、pthread、crypto、JWT 库 和 mbedtls 系列库
LIBS := -ll8w8jwt -lmbedcrypto -lmbedtls -lmbedx509 -lcrypto -lssl -lmysqlclient -lpthread -lcrypt

# 源文件列表
SRCS := config.c hashtable.c log.c ser_main.c server.c sql.c threadpool.c user.c session.c sha1.c jwt_token.c

# 目标文件列表
OBJS := $(SRCS:.c=.o)

# 目标可执行文件名
TARGET := server

.PHONY: all clean

all: $(TARGET)

# 链接目标文件生成可执行程序
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@ $(LIBS)

# 编译规则：.c 编译成 .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)


# 编译器和标志
CC := gcc
CFLAGS := -g -Wall -I../include  # 包含 include 目录中的头文件
LIBS := -lpthread -lcrypt  # 链接库

# 客户端源文件、目标文件和最终生成的可执行文件
SRCS_CLIENT := client.c threadpool.c main.c  # 添加 main.c
OBJS_CLIENT := $(SRCS_CLIENT:.c=.o)
TARGET_CLIENT := client

# 服务器源文件、目标文件和最终生成的可执行文件
SRCS_SERVER := server.c threadpool.c main.c  # 添加 main.c
OBJS_SERVER := $(SRCS_SERVER:.c=.o)
TARGET_SERVER := server

# .PHONY 声明，避免与实际文件名冲突
.PHONY: all clean

# 默认目标，先清理然后编译客户端和服务器
all: clean $(TARGET_CLIENT) $(TARGET_SERVER)

# 编译客户端
$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# 编译服务器
$(TARGET_SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# 编译 .c 文件为 .o 文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理目标文件和可执行文件
clean:
	rm -f *.o $(TARGET_CLIENT) $(TARGET_SERVER)


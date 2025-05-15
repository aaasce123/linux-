CC := gcc
CFLAGS := -g -Wall -I../include   # 包含 include 目录中的头文件
LIBS := -lpthread -lcrypt         # 链接库

# 服务器源文件、目标文件和最终生成的可执行文件
SRCS_SERVER := server.c threadpool.c ser_main.c config.c hashtable.c
OBJS_SERVER := $(SRCS_SERVER:.c=.o)
TARGET_SERVER := server

.PHONY: all clean

# 默认目标，只编译服务器（增量编译）
all: $(TARGET_SERVER)

$(TARGET_SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 手动清理目标文件和可执行文件
clean:
	rm -f *.o $(TARGET_SERVER)


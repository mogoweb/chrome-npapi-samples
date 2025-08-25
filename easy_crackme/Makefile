# Makefile for MinGW Cross-Compilation

# =============================================================================
#  配置变量 (Configuration Variables)
# =============================================================================

# 交叉编译工具链前缀。
# 'x86_64-w64-mingw32-' 用于编译 64 位 Windows 程序。
# 'i686-w64-mingw32-'  用于编译 32 位 Windows 程序。
# 使用 ?= 允许从命令行覆盖, 例如: make CROSS_PREFIX=i686-w64-mingw32-
CROSS_PREFIX ?= i686-w64-mingw32-

# 定义编译器
CC := $(CROSS_PREFIX)gcc

# 最终生成的可执行文件名
TARGET := easy_crackme.exe

# 源文件列表 (自动发现所有 .c 文件)
SRCS := $(wildcard *.c)

# C 编译选项
# -Wall -Wextra: 开启所有常用和额外的警告，有助于写出更健壮的代码。
# -O2: 优化级别2，在性能和编译时间之间取得良好平衡。
# -g: (可选) 添加调试信息，便于使用 gdb 进行调试。
CFLAGS := -Wall -Wextra -O2

# 链接选项
# -static: 静态链接 MinGW 的库，使得 .exe 文件可以独立运行，
#          无需在目标 Windows 系统上安装额外的 MinGW 运行时 DLL。
LDFLAGS := -static


# =============================================================================
#  规则定义 (Rules Definition)
# =============================================================================

# .PHONY 声明了“伪目标”，这些目标不是文件名，即使存在同名文件也会执行。
.PHONY: all clean

# 默认目标：当直接运行 `make` 时执行。
# 它依赖于最终的可执行文件。
all: $(TARGET)

# 生成最终可执行文件的规则
# $@ 代表规则的目标 (即 $(TARGET))
# $^ 代表规则的所有依赖 (即 $(SRCS))
$(TARGET): $(SRCS)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Successfully compiled ==> $(TARGET)"

# 清理规则：用于删除生成的文件
clean:
	@echo "Cleaning up..."
	@rm -f $(TARGET)
	@echo "Clean complete."
# --- 编译器和工具定义 ---
# 我们使用 i686-w64-mingw32-g++ 来编译 32 位的 Windows DLL
CXX = i686-w64-mingw32-g++
WINDRES = i686-w64-mingw32-windres
DLLTOOL = i686-w64-mingw32-dlltool
RM = rm -f

# --- 文件定义 ---
TARGET_DLL = npdemo.dll
SOURCES = dllmain.cpp log.cpp np_entry.cpp npp.cpp pch.cpp
RC_SOURCES = npdemo.rc
DEF_FILE = npdemo.def
OBJECTS = $(SOURCES:.cpp=.o) $(RC_SOURCES:.rc=.o)


# --- 编译和链接标志 ---
# -g: 添加调试信息
# -Wall: 显示所有警告
# -I.: 在当前目录查找头文件 (.h)
# -shared: 生成共享库 (DLL)
# -static-libgcc -static-libstdc++: 静态链接C和C++运行时库，使DLL不依赖外部的运行时DLL
CXXFLAGS = -g -Wall -I. -I../npapi-headers
LDFLAGS = -shared -static-libgcc -static-libstdc++
LDLIBS = -lgdi32

# --- Makefile 规则 ---

# 默认规则，第一个规则会被 'make' 命令默认执行
all: $(TARGET_DLL)

# 链接规则：如何从对象文件(.o)和定义文件(.def)生成最终的 DLL
$(TARGET_DLL): $(OBJECTS)
	@echo "==> Linking to create $(TARGET_DLL)..."
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(DEF_FILE) $(LDLIBS)
	@echo "==> Done."

%.o: %.rc
	@echo "==> Compiling resource script '$<' into object file '$@'..."
	$(WINDRES) -I. -i $< -O coff -o $@

# 编译规则：如何从 C++ 源文件(.cpp)生成对象文件(.o)
%.o: %.cpp
	@echo "==> Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理规则：用于删除编译生成的文件
clean:
	@echo "==> Cleaning up..."
	$(RM) $(OBJECTS) $(TARGET_DLL)
	@echo "==> Cleaned."

#PHONY 规则，告诉 make 'all' 和 'clean' 不是文件名
.PHONY: all clean
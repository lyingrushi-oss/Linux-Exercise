# 编译器
CC := gcc

# 编译配置
COMMON_FLAGS := -std=c11 -Wextra -Wall -O0 -g
# -MMD：生成用户头文件依赖，写入 .d 文件
# -MP：为头文件生成空的伪目标，避免删除头文件后 Make 因找不到规则而立即报错。
DEP_FLAGS := -MMD -MP
DEBUG_FLAGS := $(COMMON_FLAGS)
ASAN_FLAGS := $(COMMON_FLAGS) -fsanitize=address -fno-omit-frame-pointer

#伪目标phony target，不是我们真正想构建的目标文件，只是操作入口名字
.PHONY: all debug asan clean
#.PHONY: clean 不要把clean当文件名看到有叫clean的文件夹跳过命令，而是执行清理操作

# 构建入口:给入口起别名
all: debug asan	#默认入口
debug: build/debug/sensor_app
asan: build/asan/sensor_app

# 链接debug程序
build/debug/sensor_app: build/debug/main.o build/debug/sensor.o
	$(CC) $(DEBUG_FLAGS) $^ -o $@

# 链接asan程序
build/asan/sensor_app: build/asan/main.o build/asan/sensor.o
	$(CC) $(ASAN_FLAGS) $^ -o $@

# 编译成debug目标文件
build/debug/%.o: %.c sensor.h
	mkdir -p build/debug
	$(CC) $(DEBUG_FLAGS) $(DEP_FLAGS) -c $< -o $@

#编译成asan目标文件
build/asan/%.o: %.c sensor.h
	mkdir -p build/asan
	$(CC) $(ASAN_FLAGS) $(DEP_FLAGS) -c $< -o $@

#删除所有构建文件，make clean:执行清理动作
clean:
	rm -rf build

# -include:尝试读取这些文件；第一次构建时 .d 还不存在，也不要报错。
-include build/debug/main.d
-include build/debug/sensor.d
-include build/asan/main.d
-include build/asan/sensor.d





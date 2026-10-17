# 间接泄漏
# indirect_leak: indirect_leak.c
# 	gcc -std=c11 -Wall -Wextra -O0 -g indirect_leak.c -o indirect_leak

# still reachable
# leak_kinds: leak_kinds.c
# 	gcc -std=c11 -Wall -Wextra -O0 -g leak_kinds.c -o leak_kinds

# invalid_memory 非法访问
# invalid_memory: invalid_memory.c
# 	gcc -std=c11 -Wall -Wextra -O0 -g invalid_memory.c -o invalid_memory

# uninitialized 未初始化
uninitialized: uninitialized.c
	gcc -std=c11 -Wall -Wextra -O0 -g uninitialized.c -o uninitialized
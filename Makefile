jp_stdlib: jp_stdlib.c
	clang -o jp_stdlib jp_stdlib.c -Wall -Wextra -fsanitize=address -g

jp_string: jp_string.c
	clang -o jp_string jp_string.c -Wall -Wextra -fsanitize=address -g

#jp_stdio: jp_stdio.c
#clang -o jp_stdio jp_stdio.c -Wall -Wextra -fsanitize=address -g

#jp_stdlib: jp_stdlib.c
#clang -o jp_stdlib jp_stdlib.c -Wall -Wextra -fsanitize=address -g

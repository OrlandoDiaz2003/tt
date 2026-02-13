default:
	gcc tt.c -o tt -lncursesw
debug:
	gcc -o tt tt.c -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wunused-parameter -O0 -g3 -fsanitize=address,undefined -lncursesw

default: debug

debug:
	gcc -std=gnu17 -Og -g3 -Wall -Wextra -pedantic -fanalyzer -fsanitize=undefined chip8.c chip8_exec.c main.c -o chip8 `sdl2-config --cflags --libs`

release:
	gcc -std=gnu17 -O3 -Wall -Wextra -pedantic chip8.c chip8_exec.c main.c -o chip8 `sdl2-config --cflags --libs`

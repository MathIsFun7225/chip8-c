default: main

obj:
	mkdir -p obj

obj/%.o: %.c Makefile | obj
	gcc -lm -std=gnu17 -Og -g3 -Wall -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -pedantic -pedantic-errors \
	-fanalyzer -fsanitize=address,undefined $< -c -o $@ `sdl2-config --cflags --libs`

main: obj/main.o obj/chip8.o obj/chip8_audio.o obj/chip8_config.o obj/chip8_display.o obj/chip8_exec.o obj/chip8_run.o obj/chip8_state.o obj/helper.o Makefile | obj
	gcc -lm -std=gnu17 -Og -g3 -Wall -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -pedantic -pedantic-errors \
	-fanalyzer -fsanitize=address,undefined obj/main.o obj/chip8.o obj/chip8_audio.o obj/chip8_config.o obj/chip8_display.o obj/chip8_exec.o \
	obj/chip8_run.o obj/chip8_state.o obj/helper.o -o $@ `sdl2-config --cflags --libs`

clean:
	rm -f obj/*.o main

.PHONY: default clean

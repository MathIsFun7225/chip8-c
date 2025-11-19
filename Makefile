default: main

CFLAGS = -lm -std=gnu17 -Og -g3 -Wall -Wextra -Werror -Wno-sign-compare -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function \
         -pedantic -pedantic-errors -fanalyzer -fsanitize=undefined

obj:
	mkdir -p obj

obj/chip8_audio.o: src/chip8_audio.c src/chip8_audio.h src/chip8_config.h src/chip8_state.h Makefile | obj
	gcc $(CFLAGS) $< -c -o $@ `sdl2-config --cflags --libs`

obj/chip8_config.o: src/chip8_config.c src/chip8_config.h Makefile | obj
	gcc $(CFLAGS) $< -c -o $@

obj/chip8_display.o: src/chip8_display.c src/chip8_display.h src/chip8_config.h src/chip8_state.h Makefile | obj
	gcc $(CFLAGS) $< -c -o $@ `sdl2-config --cflags --libs`

obj/chip8_exec.o: src/chip8_exec.c src/chip8_exec.h src/chip8_config.h src/chip8_state.h Makefile | obj
	gcc $(CFLAGS) $< -c -o $@

obj/chip8_state.o: src/chip8_state.c src/chip8_state.h src/chip8_config.h src/chip8_exec.h src/helper.h Makefile | obj
	gcc $(CFLAGS) $< -c -o $@

obj/chip8.o: src/chip8.c src/chip8.h src/chip8_audio.h src/chip8_config.h src/chip8_display.h src/chip8_exec.h src/chip8_state.h Makefile | obj
	gcc $(CFLAGS) $< -c -o $@ `sdl2-config --cflags --libs`

obj/helper.o: src/helper.c src/helper.h Makefile | obj
	gcc $(CFLAGS) $< -c -o $@

obj/main.o: src/main.c src/chip8_audio.h src/chip8_config.h src/chip8_display.h src/chip8_exec.h src/chip8_state.h Makefile | obj
	gcc $(CFLAGS) $< -c -o $@ `sdl2-config --cflags --libs`

obj/test.o: src/test.c Makefile | obj
	gcc $(CFLAGS) $< -c -o $@

obj/tests.o: src/tests.c src/test.h src/chip8_config.h src/chip8_exec.h src/chip8_state.h Makefile | obj
	gcc $(CFLAGS) $< -c -o $@

main: obj/main.o obj/chip8.o obj/chip8_audio.o obj/chip8_config.o obj/chip8_display.o obj/chip8_exec.o obj/chip8_state.o obj/helper.o Makefile
	gcc $(CFLAGS) obj/main.o obj/chip8.o obj/chip8_audio.o obj/chip8_config.o obj/chip8_display.o obj/chip8_exec.o \
	obj/chip8_state.o obj/helper.o -o $@ `sdl2-config --cflags --libs`

tests: obj/tests.o obj/test.o obj/chip8_config.o obj/chip8_exec.o obj/chip8_state.o obj/helper.o Makefile
	gcc $(CFLAGS) obj/tests.o obj/test.o obj/chip8_config.o obj/chip8_exec.o obj/chip8_state.o obj/helper.o -o $@

clean:
	rm -f obj/*.o main tests

.PHONY: default clean

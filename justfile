cc := "gcc"
cflags := "-I . -Wall -Wextra -Wpedantic -fsanitize=address,undefined"

watch file:
	mkdir -p ./bin
	watchexec -f "*.c" -f "*.h" -d 500ms -r -c -- "{{cc}} {{cflags}} {{file}}.c -o ./bin/{{file}} && ./bin/{{file}}"

example file:
	mkdir -p ./bin
	watchexec -f "*.c" -f "*.h" -d 500ms -r -c -- "{{cc}} {{cflags}} examples/{{file}}.c -o ./bin/{{file}} && ./bin/{{file}}"

format:
	find . -name "*.h" -o -name "*.c" -type f | xargs clang-format --style=file --verbose -i

run file:
	mkdir -p ./bin
	{{cc}} -I . -Wall -Wextra -fsanitize=address -g -o ./bin/{{file}} {{file}}.c && ./bin/{{file}}

valgrind path_to_file:
	mkdir -p ./bin
	docker build -t cvx-valgrind -f Dockerfile.valgrind --build-arg SOURCE_FILE={{path_to_file}} .
	docker run --rm cvx-valgrind

coverage:
	mkdir -p bin coverage
	mkdir -p coverage/gcov
	find bin -name "*.gcda" -delete
	{{cc}} -g -O0 --coverage -Wall -Wextra -I . tests.c -o bin/tests
	./bin/tests
	gcovr --root . --filter 'cvx/' --html-details coverage/index.html --print-summary
	gcov -o bin/tests-tests.gcno tests.c && mv *.gcov coverage/gcov/

# setup clangd and hooks
setup:
	mkdir -p build
	mkdir -p bin
	mkdir -p bin/examples
	find . -type f -iname "*.c" | sed -r 's/^\.\/(.+)\.c$/\1/' | xargs -I {} bash -c "bear -a -o build/compile_commands.json -- {{cc}} {{cflags}} {}.c -o bin/{}"


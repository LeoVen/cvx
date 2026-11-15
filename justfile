cc := "gcc"
cflags := "-I . -Wall -Wextra -Wpedantic -ggdb -fsanitize=address,undefined -std=c23"
cflags_valgrind := "-I . -Wall -Wextra -Wpedantic -ggdb"

watch file:
	mkdir -p ./bin
	watchexec -f "*.c" -f "*.h" -d 500ms -r -c -- "{{cc}} {{cflags}} {{file}}.c -o ./bin/{{file}} && ./bin/{{file}}"

run file:
	mkdir -p ./bin
	{{cc}} {{cflags}} -o ./bin/{{file}} {{file}}.c && ./bin/{{file}}

format:
	find cvx tests -type f \( -name "*.h" -o -name "*.c" \) -not -path "*/generated/*" | xargs clang-format --style=file:./.clang-format --verbose -i

# setup clangd: compile_commands.json for the templates, using the fallback placeholders
setup:
	mkdir -p build
	bear -o build/compile_commands.json -- sh -c 'for f in cvx/templates/*.c; do {{cc}} {{cflags}} -DCVX_ENABLE_FALLBACK -fsyntax-only "$f"; done'
	python3 cvx_generator.py --config tests/config.json --compiler {{cc}} --output build/tests/generated
	bear -a -o build/compile_commands.json -- sh -c 'for f in tests/src/*.c; do {{cc}} {{cflags}} -I tests/include -I build/tests/generated/includes -fsyntax-only "$f"; done'

valgrind-init:
	docker build -t cvx-valgrind -f Dockerfile.valgrind .

# build the given sources and run them under valgrind, e.g.: just valgrind main.c generated/int_array.c
valgrind +files:
	mkdir -p ./bin
	docker run --rm -v "$(pwd):/cvx" cvx-valgrind sh -c \
		"gcc {{cflags_valgrind}} {{files}} -o bin/valgrind_target && \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 bin/valgrind_target"

docs: generate-docs
	doxygen docs/Doxyfile

generate-docs:
	python3 cvx_generator.py --config docs/config.json --compiler gcc
	clang-format --style=file:./.clang-format -i docs/includes/*.h docs/src/*.c

# run the container tests against the code generated from tests/config.json
test:
	cmake -S . -B cmake-build -DCVX_SANITIZE=ON -DCVX_COVERAGE=OFF
	cmake --build cmake-build
	ctest --test-dir cmake-build --output-on-failure

coverage:
	cmake -S . -B cmake-cov -DCVX_SANITIZE=OFF -DCVX_COVERAGE=ON
	cmake --build cmake-cov
	ctest --test-dir cmake-cov --output-on-failure
	# TODO: this is MacOS only
	gcovr --root . --gcov-executable "xcrun llvm-cov gcov" --filter cmake-cov/tests/generated/src/ --html-details cmake-cov/coverage.html


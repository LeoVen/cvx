cc := "gcc"
cflags := "-I . -Wall -Wextra -Wpedantic -ggdb -fsanitize=address,undefined"
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

valgrind-init:
	docker build -t cvx-valgrind -f Dockerfile.valgrind .

# build the given sources and run them under valgrind, e.g.: just valgrind main.c generated/int_array.c
valgrind +files:
	mkdir -p ./bin
	docker run --rm -v "$(pwd):/cvx" cvx-valgrind sh -c \
		"gcc {{cflags_valgrind}} {{files}} -o bin/valgrind_target && \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 bin/valgrind_target"

# generate cvx container types from a config
# compiler is mandatory, e.g.: just cvx-generate gcc tests/config.json
cvx-generate compiler config:
	python3 cvx_generator.py --config {{config}} --compiler {{compiler}}

docs: generate-docs
	doxygen docs/Doxyfile

generate-docs:
	python3 cvx_generator.py --config docs/config.json --compiler gcc
	clang-format --style=file:./.clang-format -i docs/includes/*.h docs/src/*.c

# run the cvx generator's own unit + end-to-end tests
cvx-test:
	python3 -m unittest discover -s tests/generator -p "test_*.py" -v

generate-tests:
	python3 cvx_generator.py --config tests/config.json --compiler {{cc}}

# run the container tests against the code generated from tests/config.json
test: generate-tests
	mkdir -p bin
	{{cc}} {{cflags}} -I tests/includes tests/tests.c tests/src/*.c -o bin/tests
	./bin/tests

# coverage of the generated containers exercised by tests/tests.c
coverage: generate-tests
	mkdir -p bin coverage
	find bin -name "*.gcda" -delete
	{{cc}} -g -O0 --coverage -Wall -Wextra -I . -I tests/includes tests/tests.c tests/src/*.c -o bin/tests_cov
	./bin/tests_cov
	gcovr --root . --filter 'tests/src/' --html-details coverage/index.html --print-summary

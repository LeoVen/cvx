watch file:
	mkdir -p ./bin
	watchexec -f "*.c" -f "*.h" -d 500ms -r -c -- "gcc -Wall -Wextra -I . {{file}}.c -o ./bin/{{file}} && ./bin/{{file}}"

example file:
	watchexec -f "*.c" -f "*.h" -d 500ms -r -c -- "gcc -Wall -Wextra -I . examples/{{file}}.c -o ./bin/{{file}} && ./bin/{{file}}"

format:
	find . -name "*.h" -o -name "*.c" -type f | xargs clang-format --style=file --verbose -i

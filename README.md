# cvx

A modern C library of generic data structures that are generated based on templates.
The resulting code is a clean (of macros) and type safe C code that can be inspected.

---

## Quick start

Describe the containers you want in a JSON config:

```json
{
  "instantiations": [
    {
      "template": "dynamic_array",
      "value_type": "int",
      "struct_name": "int_array",
      "prefix": "ia",
      "out_dir": "generated"
    }
  ]
}
```

Run the generator. `--compiler` is the C compiler used to expand the templates (`gcc`, `clang` or `cc`):

```sh
python3 cvx2/generator/generator.py --config config.json --compiler gcc
# wrote generated/int_array.h
# wrote generated/int_array.c
```

`out_dir` is resolved relative to the config file. The output is plain C: no template macros are left in it, so it reads like hand-written code and works with any LSP.

Use it like any other header/source pair:

```c
#include <stdio.h>

#include "generated/int_array.h"

static int int_comp(int const *a, int const *b)
{
    return (*a > *b) - (*a < *b);
}

int main(void)
{
    struct int_array_vtabv vtabv = { .comp = int_comp };
    struct int_array arr;

    if (ia_init(&arr, &vtabv, 0) != CVX_FLAG_OK)
        return 1;

    ia_push(&arr, ia_count(&arr), 30);
    ia_push(&arr, ia_count(&arr), 10);
    ia_push(&arr, ia_count(&arr), 20);
    ia_sort(&arr);

    for (size_t i = 0; i < ia_count(&arr); i++)
    {
        int value;
        ia_get(&arr, i, &value);
        printf("%d\n", value);
    }

    ia_drop(&arr);
    return 0;
}
```

The generated files include `cvx2/core.h` and `cvx2/flags.h`, so compile with the repository root on the include path:

```sh
gcc -I path/to/cvx main.c generated/int_array.c -o main
```

### Configuration

Each entry in `instantiations` generates one `.h`/`.c` pair. The full schema is in `cvx2/generator/config.schema.json`.

| Field | Required | Description |
|---|---|---|
| `template` | yes | `dynamic_array`, `hashtable`, etc. See [Templates](#templates)|
| `struct_name` | yes | Name of the generated struct, also the prefix of its helper structs (e.g. `int_array_vtabv`) |
| `prefix` | yes | Prefix of every generated function (e.g. `ia` gives `ia_push`) |
| `value_type` | depends | Element type. Depends on the data structure |
| `key_type` | depends | Key type. Depends on the data structure |
| `file_name` | no | Basename of the generated files and include guard. Defaults to `struct_name` |
| `case` | no | Naming case, see [Case](#case). Defaults to snake_case for both functions and structs |
| `variants` | depends | Template-specific choices, see [Templates](#templates) |
| `includes` | no | Headers to include at the top of the generated `.h`, needed for any non-builtin type. `"<stdint.h>"` is emitted as a system include, anything else as a quoted path |
| `out_dir` | no | Output directory, relative to the config file. Defaults to `.` |

---

## Limitations

### Key and value types

`value_type` and `key_type` are spliced into the templates as plain text, so they must be types that can be declared as `T name;` and assigned with `=`.

- **Raw array and function pointer types are rejected.** `int[10]` or `int (*)(int, int)` can't be used as a declarator prefix (the generator would emit `int[10] item;`). Wrap them in a typedef instead and add the header that defines it to `includes`:

  ```c
  typedef struct { int items[10]; } int10_t; // fixed-size array
  typedef int (*binop_fn)(int, int);         // function pointer
  ```

- **Array typedefs don't work.** `typedef int arr10[10];` arrays can't be assigned, so the generated code won't compile. Wrap the array in a struct as above.
- **`const`-qualified types don't work.** e.g. `const int` or `char *const`. The containers assign into their slots, which a `const` type forbids. A pointer *to* const data is fine (`const char *`).
- **Non-builtin types need `includes`.** Each generated `.c` is its own translation unit and only includes its own generated `.h`, so any struct, enum or typedef used as a type must have its header listed in the instantiation's `includes`.

### Callbacks

Callbacks receive elements by pointer (`T const *` for read-only access, `T *` for `drop`/the copy destination).

- **`const` is shallow.** `T const *` stops a callback from modifying the stored element itself, but not what it points to. For `T = char *` a comparator receives `char *const *`: it can't repoint the stored pointer, but it can still write to the characters.
- **The hash function takes no seed or context.** `size_t (*)(T const *)` has no per-table state, so seeding needs to use a global state.

---

## Templates

Templates live in `cvx2/templates/` as a `.h`/`.c` pair written in ordinary C, with placeholder macros where the types and names go. To generate an instantiation, the generator:

1. Turns the config into `-D` defines (`CVX_VAL=int`, `CVX_SNAME=int_array`, ...).
2. Pulls out the `#include` lines so the preprocessor doesn't inline other headers.
3. Runs the compiler's preprocessor (`-E -C`, keeping comments) over the template, which resolves every placeholder and selects the `#if` branches for the chosen variants.
4. Puts the includes back, adds an include guard and writes the result.

Every template includes `cvx2/fallback.h`, which defines default placeholder values when `CVX_ENABLE_FALLBACK` is set. This lets clangd understand a template opened directly, it has no effect on generation.

- `dynamic_array`: A growable array
  - variants: None
- `circular_buffer`: A fixed-capacity ring buffer. Pushing onto a full buffer overwrites the element at the opposite end
  - variants: None
- `linked_list`: A linked list owning its nodes. Nodes are public, so a `struct ..._node *` can be used as a position for O(1) inserts and removals
  - node: `single` or `double` (required)
- `binary_heap`: A priority queue (max-heap ordered by `comp`; reverse `comp` for a min-heap)
  - variants: None
- `hashtable`: A hashtable
  - kind: `map` (default) or `set`
  - collision: `open_addressing` or `separate_chaining`
- `hamt`: Hash array mapped tree
  - variants: None

### Macros

These are the placeholders a template uses. The generator defines them from the config.

| Macro | Required | Description |
|---|---|---|
| `CVX_VAL` | value containers | The value type stored in the container (e.g. `int`, `char *`, `struct point`) |
| `CVX_KEY` | key-value containers | The key type |
| `CVX_SNAME` | yes | The name of the generated struct (e.g. `int_array` becomes `struct int_array`) |
| `CVX_PFX` | yes | Prefix for all generated functions (e.g. `ia` becomes `ia_push(...)`) |
| `CVX2_*` | depends | Data structures can have more options |

### Case

`case` sets the naming style of the suffixes the generator appends, separately for functions and structs:

```json
"case": { "function": "snake_case", "struct": "PascalCase" }
```

Either key can be omitted and defaults to `snake_case`. Only the suffixes are converted: `struct_name` and `prefix` are used exactly as written, and the suffix is joined to them directly.

| Case | Function (`prefix: "ia"`) | Struct (`struct_name: "IntArray"`) |
|---|---|---|
| `snake_case` | `ia_push`, `ia_index_of` | `IntArray_vtabv` |
| `camelCase` | `iapush`, `iaindexOf` | `IntArrayvtabV` |
| `PascalCase` | `iaPush`, `iaIndexOf` | `IntArrayVtabV` |

Suffix spellings come from `cvx2/names.h`.
Private functions do not follow this casing and are always written as they are, e.g. `FUNC(__assert_capacity)` with prefix `da` gives `da__assert_capacity`.

---

## Error handling

Every operation that can fail returns an `enum cvx_flags`, and results are written through out-pointers:

```c
int value;
enum cvx_flags flag = ia_get(&arr, 99, &value);
if (flag != CVX_FLAG_OK)
    fprintf(stderr, "%s: %s\n", cvx_flags_str[flag], cvx_flags_description[flag]);
// RANGE: Index out of range
```

`cvx_flags_str` and `cvx_flags_description` are indexed by flag value.

| Flag | Value | Meaning |
|---|---|---|
| `CVX_FLAG_OK` | 0 | No error |
| `CVX_FLAG_WRONG_TAG` | 1 | A container was passed to a function for a different type |
| `CVX_FLAG_ALLOC` | 2 | A memory allocation failed |
| `CVX_FLAG_EMPTY` | 3 | Operation requires elements but the container is empty |
| `CVX_FLAG_FULL` | 4 | Container has reached capacity and does not resize |
| `CVX_FLAG_RANGE` | 5 | Index is out of bounds |
| `CVX_FLAG_NOT_FOUND` | 6 | Key or value not found |
| `CVX_FLAG_INVALID` | 7 | Invalid argument or operation |
| `CVX_FLAG_DUPLICATE` | 8 | Duplicate key or value |
| `CVX_FLAG_ERROR` | 9 | Generic or unknown error |
| `CVX_FLAG_VTAB` | 10 | A required vtab callback was not provided (e.g. `comp` for `sort`) |

---

## Installing dependencies

Required tooling

- python3 (3.10+)
- gcc or clang
- clang-format
- valgrind
  - If you are not on linux, you can use the provided Dockerfile (you'll need docker)
- gcovr
  - `pip3 install gcovr`
- watchexec
  - `brew install watchexec`
  - `cargo install watchexec`
- bear
  - `brew install bear`

## Running stuff

The project uses [`just`](https://github.com/casey/just) as a command runner and [`watchexec`](https://github.com/watchexec/watchexec) for live reloading.

```sh
# Run and watch a top-level .c file (e.g. main.c -> ./bin/main)
just watch main

# Run and watch an example (e.g. examples/hanoi.c -> ./bin/hanoi)
just example hanoi

# Auto-format all .c and .h files with clang-format
just format

# Generate cvx2/examples/config.json
just cvx2-generate gcc

# Generate cvx2/docs/config.json and build the Doxygen docs into docs/output
just docs
```

---

## Testing

```sh
just cvx2-test
```

This runs the generator's unit tests in `cvx2/generator/tests/`. The end-to-end test generates `cvx2/examples/config.json`, compiles the output together with `cvx2/examples/smoke.c` and runs it, so it needs `gcc`, `clang` or `cc` on `PATH`.

The legacy `cvx/` library has its own tests in `tests/`, included from `tests.c`:

```sh
gcc -Wall -Wextra -I . tests.c -o bin/tests && ./bin/tests
```

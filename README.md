# cvx

A generic C data structure library built on templating header files.

There are two main types of data structures:

**Implementations** are data structures that provide a wide range of operations, even if they are sometimes slow. These are generalist containers.

**Interfaces** are abstractions that can be implemented by multiple data structures. They define a set of functions in a `vtable` and you can "cast" an implementation type to an interface type.

---

## Quick start

```c
#include <stdio.h>

// 1. Define the interface type
#define V         int             // data type for the stack interface
#define INTERFACE stack           // name of the struct
#include "cvx/istack.h"

// 2. Instantiate an implementation
#define V          int            // data type for my dynamic array implementation
#define SNAME      my_list        // name of my dynamic array (e.g. struct my_list)
#define PFX        ml             // prefix to all functions
#define TAG        1              // unique tag for this implementation
#define IMPL_STACK stack          // generates the ml_as_stack() "cast"
#include "cvx/dynamic_array.h"

#include "cvx/interface_macros.h" // utility macros when working with interfaces

int main(void)
{
    struct stack s = ml_as_stack(ml_new_with(8));

    cvx_push(&s, 10);
    cvx_push(&s, 20);
    cvx_push(&s, 30);

    int out;
    cvx_pop(&s, &out);            // out == 30
    printf("%d\n", cvx_peek(&s)); // 20

    cvx_drop(&s);
}
```

Check out the examples folder to see how to use the library.

---

## Available Implementations and Interfaces

### Available implementations

| Header | Description |
|---|---|
| `cvx/dynamic_array.h` | Growable contiguous array. O(1) amortized push/pop at back, O(n) at front. |
| `cvx/slinked_list.h` | Singly linked list with head and tail pointers. O(1) push/pop at front; O(n) at back. |

### Available interfaces

| Header | Interface operations |
|---|---|
| `cvx/istack.h` | `push`, `pop`, `peek`, `replace`, `count`, `new`, `clone`, `drop` |

---

## Implementations

An implementation is instantiated by defining a set of macros and then including a header. Each `#include` produces a new and independent concrete type.

### Macros

| Macro | Required | Description |
|---|---|---|
| `V` | yes | The value type stored in the container (e.g. `int`, `char *`, `struct point`) |
| `K` | key-value only | The key type (same as `V` but for key-value containers) |
| `SNAME` | yes | The name of the generated struct (e.g. `my_list` becomes `struct my_list`) |
| `PFX` | yes | Prefix for all generated functions (e.g. `ml` becomes `ml_push_back(...)`) |
| `TAG` | yes | A unique integer that identifies this container type at runtime. Must be distinct from every other `TAG` in the same program. |

All macros are automatically undefined after the `#include` via `undef.h`, so there is no risk of them leaking into subsequent templates.

### User type functions

For data structures that need to compare, copy, hash, or free elements, define the relevant macros before the `#include`. Which macros are required depends on the data structure.

```c
#define V_COMP my_comp // int    (*comp)(V, V) — required for ordered structures
#define V_COPY my_copy // V      (*copy)(V)    — optional for deep clone
#define V_DROP my_drop // void   (*drop)(V)    — required when elements own heap memory
#define V_HASH my_hash // size_t (*hash)(V)    — required for hash-based structures
#define V_PRIO my_prio // int    (*prio)(V, V) — required for priority queues
```

For key-value structures, the same set applies to the key type under `K_`:

```c
#define K_COMP my_key_comp
#define K_COPY my_key_copy
#define K_DROP my_key_drop
#define K_HASH my_key_hash
#define K_PRIO my_key_prio
```

## Interfaces

An interface is a `struct` type consisting of an `instance` pointer and a `vtable` pointer. Multiple data structures can implement one interface.

Usually, when working with this pattern, you have `void *` pointers. This library instead uses a `cvx_container` type, which holds extra information for error handling.

### Declaring an interface

```c
#define V         int
#define INTERFACE stack
#include "cvx/istack.h"
// produces: struct stack { cvx_container *instance; struct stack_vtable *vtable; }
```

### Casting an implementation to an interface

Define `IMPL_<INTERFACE>` when you want to be able to cast an implementation to an interface:

```c
#define V         int
#define SNAME     my_list
#define PFX       ml
#define TAG       1
#define IMPL_STACK stack   // must match the INTERFACE name declared above
#include "cvx/dynamic_array.h"
// generates: struct stack ml_as_stack(cvx_container *instance)
```

Then cast at construction time:

```c
struct stack s = ml_as_stack(ml_new_with(8));
```

The same interface can back different implementations simultaneously:

```c
struct stack array_stack  = ml_as_stack(ml_new_with(8));
struct stack linked_stack = sl_as_stack(sl_new());
// both satisfy `struct stack` — pass either to the same function
```

### Interface macros

```c
#include "cvx/interface_macros"
```

It is annoying to call functions from interfaces via the `vtable`. For example, given that `s` is a stack, to push an item you need to: `s->vtable->push(s->instance, item)`.

Instead you can use these macros and simply: `cvx_push(s, item)`. Neat!

## Error handling

Every `cvx_container` carries a `flag` field of type `enum cvx_flags`. Functions set this field instead of returning error codes, so the caller can inspect it after any operation.

```c
sc_pop_back(col, &out);
if (col->flag != CVX_FLAG_OK) { /* handle error */ }

// or via an interface:
cvx_pop(&s, &out);
if (cvx_flag(&s) != CVX_FLAG_OK) { /* handle error */ }
```

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


### Tags and `CVX_FLAG_WRONG_TAG`

Every implementation must be given a **unique `TAG`** integer. The tag is embedded in `cvx_container.tag` at construction and checked at the start of every function. Passing the wrong container to a function (e.g. a linked list where a dynamic array is expected) sets `CVX_FLAG_WRONG_TAG` and returns immediately rather than corrupting memory.

---

## Building and running

There is no build process. This is a header-only library.

The project uses [`just`](https://github.com/casey/just) as a command runner and [`watchexec`](https://github.com/watchexec/watchexec) for live reloading.

```sh
# Run and watch a top-level .c file (e.g. main.c → ./bin/main)
just watch main

# Run and watch an example (e.g. examples/hanoi.c → ./bin/hanoi)
just example hanoi

# Auto-format all .c and .h files with clang-format
just format
```

---

## Testing

Tests live in `tests/` as header-only files and are included in `tests.c`.

```sh
gcc -Wall -Wextra -I . tests.c -o bin/tests && ./bin/tests
```

The test framework is located in `tests/cvxtest.h`. Check it out if you want to know more.


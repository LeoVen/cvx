# cvx

A generic C data structure library built on templating header files.

There are two main types of data structures:

**Implementations** are data structures that provide a wide range of operations, even if they are sometimes slow. These are generalist containers.

**Interfaces** are abstractions that can be implemented by multiple data structures. They define a set of functions in a `vtable` and you can "cast" an implementation type to an interface type.

---

## Quick start

Check out the examples folder to see more examples.

```c
#include <stdio.h>

// 1. Define the interface type
#define V         int             // data type for the stack interface
#define INTERFACE stack           // name of the struct
#include "cvx/interface/stack.h"

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
    struct stack s = ml_as_stack(ml_new_with(NULL, 8));

    cvx_push(&s, 10);
    cvx_push(&s, 20);
    cvx_push(&s, 30);

    int out = cvx_pop(&s);
    printf("Pop: %d\nPeek: %d\n", out, cvx_peek(&s));

    cvx_drop(&s);
}
```

---

## Available Implementations and Interfaces

### Available implementations

| Header | Description | Interfaces | Iterators |
|---|---|---|---|
| `cvx/dynamic_array.h` | Growable contiguous array. O(1) amortized push/pop at back, O(n) at front. | `stack` | `random_access_iterator` |
| `cvx/slinked_list.h` | Singly linked list with head and tail pointers. O(1) push/pop at front; O(n) at back. | `stack`, `queue` | `forward_iterator` |

### Available interfaces

| Header | Operations |
|---|---|
| `cvx/interface/stack.h` | `push`, `pop`, `peek`, `replace`, `count`, `new`, `clone`, `drop` |
| `cvx/interface/queue.h` | `enqueue`, `dequeue`, `count`, `new`, `clone`, `drop` |

### Available iterators

| Header | Operations |
|---|---|
| `cvx/iter/forward_iterator.h` | `start`, `drop`, `at_start`, `at_end`, `count`, `to_start`, `next`, `forward`, `value`, `index` |
| `cvx/iter/bidirectional_iterator.h` | `start`, `end`, `drop`, `at_start`, `at_end`, `count`, `to_start`, `to_end`, `next`, `prev`, `forward`, `backward`, `value`, `index` |
| `cvx/iter/random_access_iterator.h` | `start`, `end`, `drop`, `at_start`, `at_end`, `count`, `to_start`, `to_end`, `next`, `prev`, `forward`, `backward`, `go_to`, `value`, `index` |

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

### User type virtual tables

Each instantiated container generates a `struct <SNAME>_vtabv` type that holds optional (some times mandatory) function pointers for element operations. You create one and pass it to the constructor. A `NULL` value is allowed when no callbacks are needed.

```c
// The generated vtabv struct has these fields
struct my_list_vtabv {
    int    (*comp)(V, V);   // compare two elements - required for ordered structures
    V      (*copy)(V);      // deep-copy an element - optionally called by clone()
    void   (*drop)(V);      // free an element      - optionally called by drop() and clear()
    size_t (*hash)(V);      // hash an element      - required for hash-based structures
    int    (*prio)(V, V);   // compare priorities   - required for priority queues
};
```

**Example - dynamic array of heap-allocated strings:**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define V         char *
#define SNAME     str_array
#define PFX       sa
#define TAG       1
#include "cvx/dynamic_array.h"

char *str_copy(char *s) { return strdup(s); }
void  str_drop(char *s) { free(s); }

int main(void)
{
    // Build a vtabv with copy and drop - no comp/hash/prio needed here
    struct str_array_vtabv cbs = { .copy = str_copy, .drop = str_drop };

    cvx_container *col = sa_new_with(&cbs, 8);

    sa_push_back(col, strdup("hello"));
    sa_push_back(col, strdup("world"));

    // clone() calls str_copy on each element - original and clone are independent
    cvx_container *clone = sa_clone(col);

    printf("%s %s\n", sa_get(col, 0), sa_get(clone, 1));

    sa_drop(col);   // calls str_drop on each element, then frees the array
    sa_drop(clone);
}
```

For key-value structures a matching `struct <SNAME>_vtabk` is generated for the key type, with the same set of fields.

## Interfaces

An interface is a `struct` type consisting of an `instance` pointer and a `vtable` pointer. Multiple data structures can implement one interface.

Usually, when working with this pattern, you have `void *` pointers. This library instead uses a `cvx_container` type, which holds extra information for error handling.

### Declaring an interface

```c
#define V         int
#define INTERFACE stack
#include "cvx/intf/stack.h"
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
// both satisfy `struct stack` - pass either to the same function
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

## Installing dependencies

Required tooling

- gcc
- clang
- clang-format
- valgrind
  - If you are not on linux, you can use the provided Dockerfile (you'll need docker)
- gcovr
  - `pip3 install gcovr`
- watchexec
  - `brew install watchexec`
  - `cargo install watchexec`

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

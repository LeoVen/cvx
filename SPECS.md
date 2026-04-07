TODO: clean this up a bit more

# cvx - Library Specifications

This document describes the conventions, patterns, and structural rules of the cvx library. It is intended as a reference for contributors and as context for AI agents generating new code.

- [cvx - Library Specifications](#cvx---library-specifications)
  - [Overview](#overview)
  - [Implementations](#implementations)
  - [Interfaces](#interfaces)
    - [Interface casting](#interface-casting)
  - [Iterators](#iterators)
  - [Main folders](#main-folders)
  - [Macro conventions](#macro-conventions)
    - [Implementation headers](#implementation-headers)
    - [Interface and Iterator headers](#interface-and-iterator-headers)
  - [Naming conventions](#naming-conventions)
    - [Function names](#function-names)
    - [Struct names](#struct-names)
    - [Parameter naming](#parameter-naming)
  - [Common Function Conventions](#common-function-conventions)
    - [Initializers and Destructors](#initializers-and-destructors)
    - [Iterators](#iterators-1)
  - [cvx\_container](#cvx_container)
    - [Tags](#tags)
    - [Flag](#flag)
  - [CVX\_CONTAINER\_GUARDS](#cvx_container_guards)
  - [Function categories and ordering](#function-categories-and-ordering)
  - [VTAB\_V and VTAB\_K - per-instance callbacks](#vtab_v-and-vtab_k---per-instance-callbacks)
  - [Error handling](#error-handling)
  - [Test structure](#test-structure)
    - [File organisation](#file-organisation)
    - [Test file naming](#test-file-naming)
    - [Test function naming](#test-function-naming)
    - [Test suite runner naming](#test-suite-runner-naming)
    - [Shared test infrastructure](#shared-test-infrastructure)
    - [Test categories and what to cover](#test-categories-and-what-to-cover)
  - [Adding a new implementation - checklist](#adding-a-new-implementation---checklist)

---

## Overview

cvx is a header-only, generic C data structure library built on a C preprocessor templating pattern. Each header file is a template: the caller defines a set of macros, includes the header, and gets a fully concrete, type-safe struct and its associated functions. Each `#include` is one independent instantiation.

There are three kinds of header files:

- **Implementations** (`cvx/*.h`) - concrete data structures and iterators (`dynamic_array`, `slinked_list`, etc.)
- **Interfaces** (`cvx/interface/*.h`) - abstract operation sets (`stack`, `queue`, etc.)
- **Iterators** (`cvx/iter/*.h`) - abstract traversal protocols (`forward_iterator`, `random_access_iterator`, etc.)

---

## Implementations

TODO: write me

## Interfaces

An interface is declared by defining `V` and `INTERFACE` and including the interface header. It generates a `struct` type consisting of an `instance` pointer and a `vtable` pointer. Multiple data structures can implement one interface.

```c
#define V         int
#define INTERFACE stack_int
#include "cvx/interface/stack.h"
// produces: struct stack_int { cvx_container *instance; struct stack_int_vtable *vtable; }
```

Usually, when working with this pattern, you have `void *` pointers. This library instead uses a `cvx_container` type, which holds extra information for error handling.

### Interface casting

An implementation exposes a cast function by defining `IMPL_<INTERFACE>` before its `#include`:

```c
#define V int
#define SNAME my_list
...
#define IMPL_STACK stack_int
#include "cvx/dynamic_array.h" // dynamic_array's data structure can implement a stack
// generates: struct stack_int da_int_as_stack_int(cvx_container *instance)
```

The cast function is named `<PFX>_as_<INTERFACE>` (for containers) or `<PFX>_iter_as_<INTERFACE>` (for iterators).

A global vtable is created that allows the implementation type to be cast as the interface. This global variable is named `cvx_vtables_<SNAME>_as_<INTERFACE>` and is a static struct populated by the `_cast.h` header.

---

## Iterators

Iterators are generated along with their implementation types. Much like how implementation types can be casted to interfaces, iterators can also be casted to iterator interfaces.

An iterator interface is declared and used the same way as other interfaces:

```c
#define V         int
#define INTERFACE forward_iter
#include "cvx/iter/forward_iterator.h"

// In the implementation:
#define V int
#define SNAME my_linked_list
...
#define IMPL_FORWARD_ITER forward_iter
#include "cvx/slinked_list.h" // slinked_list's iterator can implement a forward_iterator
// generates: struct forward_iter sll_int_iter_as_forward_iter(cvx_container *instance)
```

Iterator structs always begin with `cvx_container super` as their first member and use `ITER_TAG` as their tag.

---

## Main folders

- `cvx/` - folder containing all library implementation
- `examples/` - folder with at least one example per implementation, interface or iterator
- `tests/` - where all test cases are defined and separated by folders mirroring the `cvx` folder

---

## Macro conventions

### Implementation headers

The following are macros need to be defined before an `#include` to properly generate code.

| Macro | Required | Description |
|---|---|---|
| `V` | yes | Value type stored in the container (e.g. `int`, `char *`) |
| `K` | key-value only | Key type for key-value containers |
| `SNAME` | yes | Name for the generated struct (e.g. `my_list` becomes `struct my_list`) |
| `PFX` | yes | Prefix for all generated functions (e.g. `ml` becomes `ml_push_back(...)`) |
| `TAG` | yes | Unique integer that identifies this container type at runtime |

The following macros can also defined before an `#include`, but only for implementation header files.
These are so-called `impl` macros. It specifies the struct name of a previously-declared interface.

| Macro | Description |
|---|---|
| `IMPL_STACK` | Name of a previously-declared `stack` interface to cast to |
| `IMPL_QUEUE` | Same, for `queue` |
| `IMPL_FORWARD_ITER` | Name of a `forward_iterator` interface to cast to |
| `IMPL_RANDOM_ACCESS_ITER` | Same, for `random_access_iterator` |
| `IMPL_BIDIRECTIONAL_ITER` | Same, for `bidirectional_iterator` |

Internal macros (defined inside the header, not by the user):

| Macro | Expands to |
|---|---|
| `FUNC(X)` | `CVX_(PFX, X)` - used to name every generated function |
| `NODE` | `CVX_(SNAME, _node)` - node struct name (linked structures) |
| `ITERATOR` | `CVX_(SNAME, _iter)` - iterator struct name |
| `ITER_TAG` | `TAG * CVX_ITER_TAG_MULT` - tag for the iterator of this container |
| `VTAB_V` | `CVX_(SNAME, _vtabv)` - name of the generated vtabv struct |
| `VTAB_K` | `CVX_(SNAME, _vtabk)` - name of the generated vtabk struct |

All macros are automatically `#undef`-ed at the end of every implementation header via `#include "cvx/undef.h"`. They never leak to subsequent templates.

### Interface and Iterator headers

| Macro | Required | Description |
|---|---|---|
| `V` | yes | Value type stored in the container (e.g. `int`, `char *`) |
| `K` | key-value only | Key type for key-value containers |
| `INTERFACE` | yes | Name for the generated struct (e.g. `my_stack` becomes `struct my_stack`, `my_iterator` becomes `struct my_iterator`) |

---

## Naming conventions

### Function names

- All generated function are prefixed by `PFX` + `_`
- Iterator functions are always prefixed by `PFX` + `_iter_`
- Private helper functions use a double-underscore separator: e.g. `PFX` + `__assert_capacity`

### Struct names

- Generated structs are prefixed by `SNAME` for implementation headers
- For interface headers, generated structs are prefixed by `INTERFACE`

### Parameter naming

Internal parameters use surrounding underscores to avoid shadowing user-visible names or conflicting with macros:

| Role | Name |
|---|---|
| The container | `_col_` or `_self_` |
| Original in clone | `_orig_` |
| Copy in clone | `_copy_` |
| Results | `_res_` |
| Item value | `_val_` |
| Key | `_key_` |
| New value (replace) | `new` |
| Old value (replace return) | `_old_` |
| Popped value | `_val_` |
| vtabv argument | `_vtabv_` |
| vtabk argument | `_vtabk_` |
| Iterator | `_iter_` |
| Target (iter constructor) | `_target_` |

---

## Common Function Conventions

### Initializers and Destructors

Every implementation provides a consistent set of lifecycle functions. Their names encode both their allocation strategy and their intent.

- **`_init`**
  - Initializes a container on the stack and returns it by value
  - Accepts an optional `vtabv` or `vtabk`
  - Use this when you want to manage the memory yourself or store the container inline in a struct
  - Similar to `_new`
- **`_init_with`**
  - Same signature as `_init`, but accepts additional construction arguments
  - Only available when a data structure may have additional arguments
  - Similar to `_new_with`
- **`_new`**
  - Heap-allocates a container and sets all defauls
  - Returns a `cvx_container *`
  - Similar to `_init`
  - Used by interfaces
- **`_new_with`**
  - Heap-allocates a container
  - Returns a `cvx_container *`
  - Similar to `_init_with`
- **`_copy`**
  - Creates a deep copy of a container and returns it by value
  - Elements are optionally copied using `vtabv->copy`, otherwise a shallow copy is performed
  - The result is independent of the original
- **`_clone`**
  - Creates a deep copy of a heap-allocated container and returns a new `cvx_container *`
  - Elements are optionally copied using `vtabv->copy`, otherwise a shallow copy is performed
  - Copies the `vtabv` pointer from the original so callbacks remain active on the clone
  - Used by interfaces
- **`_drop`**
  - Frees all internal resources (nodes, buffers, etc.)
  - Optionally frees each element if `vtabv->drop` is defined
  - Frees the passed `cvx_container` pointer
  - After calling `_drop`, the pointer is freed and invalid
  - Used by interfaces
- **`_clear`**
  - Frees all internal resources and resets the container to an empty, but valid, state
  - Does not free the `cvx_container` pointer passed to it
  - The container remains valid and can be reused
  - Resets `count` to 0, nulls internal pointers, and sets `flag = CVX_FLAG_OK`

When to use each function:

- `_init*` functions are stack-allocated and return by value
- `_new*` functions are heap-allocated and return `cvx_container *`
- `_drop` pairs with `_new*`
- `_copy` pairs with `_init*`
- `_clone` pairs with `_new*`
- `_clear` can be used with either

Summary Table:

| Function | Input | Output | Used in interfaces | Paired destructor |
|---|---|---|---|---|
| `_init` | - | stack value | No | `_clear` |
| `_init_with` | - | stack value | No | `_clear` |
| `_new` | - | heap pointer | Yes | `_drop` |
| `_new_with` | - | heap pointer | No | `_drop` |
| `_copy` | either | stack value | No | `_clear` |
| `_clone` | either | heap pointer | Yes | `_drop` |
| `_drop` | heap pointer | - | Yes | - |
| `_clear` | either | - | No | - |

### Iterators

TODO: Iterators also follow a common interface and inheritance structure.

---

## cvx_container

Every container struct has `cvx_container super` as its **first member**. This allows safe casting between `struct SNAME *` and `cvx_container *`.

```c
typedef struct cvx_container {
    size_t         tag;   // identifies the type; checked by every function
    enum cvx_flags flag;  // result of the last operation
} cvx_container;
```

### Tags

Iterator tags are always `TAG * CVX_ITER_TAG_MULT` (default multiplier: 100). A container with `TAG 99` gets iterator tag `9900`. Tags must be unique across all instantiations in a translation unit.

### Flag

`flag` is set to `CVX_FLAG_OK` on success and to the appropriate error code on failure. Functions never return error codes. Callers must inspect `col->flag`.

---

## CVX_CONTAINER_GUARDS

Every function that takes a `cvx_container *` begins with this guard:

```c
CVX_CONTAINER_GUARDS(TAG, _col_, <error_return>);
```

This expands to:

```c
if (_col_->tag != TAG) {
    _col_->flag = CVX_FLAG_WRONG_TAG;
    return <error_return>;
}
```

The `<error_return>` values by convention:

- `void` functions use ` ` (empty)
- functions returning a pointer use `NULL`
- functions returning `size_t` use `0`
- functions returning `bool` use `false`
- functions returning `V` use `(V){ 0 }`
- functions returning `K` use `(K){ 0 }`

---

## Function categories and ordering

Inside each implementation header, declarations and then definitions appear in this fixed order:

1. **Internal macro definitions** (`FUNC`, `NODE`, `ITERATOR`, `ITER_TAG`, `VTAB_V`)
2. **Struct definitions** - vtabv, node (if applicable), container, iterator
3. **Forward declarations** - all functions declared before any are defined
4. **Non-allocating initializers** - `_init()`, `_init_with()`, `_copy()` (stack-allocated, return by value)
5. **Allocating initializers** - `_new()`, `_new_with()`, `_clone()` (heap-allocated, return `cvx_container *`)
6. **Destructors** - `_drop()`, `_clear()`
7. **Getters** - `_count()`, `_capacity()`, `_empty()`, `_full()`, `_front()`, `_back()`, `_get()`
8. **Mutators** - `_push_front()`, `_push_back()`, `_push_at()`, `_pop_front()`, `_pop_back()`, `_pop_at()`, `_replace_front()`, `_replace_back()`
9. **Iterator section** (after a comment block `/// ITERATOR ///`) - `_iter_init_start()`, `_iter_init_end()`, `_iter_start()`, `_iter_end()`, `_iter_drop()`, then state functions, then movement functions, then access functions
10. **Private functions** (after a comment block `/// PRIVATE FUNCTIONS ///`)
11. **Interface cast blocks** - one `#ifdef IMPL_<INTERFACE>` block per supported interface, each including the corresponding `_cast.h`
12. **Cleanup** - `#undef VTAB_V` (and any other local macros), then `#include "cvx/undef.h"`

---

## VTAB_V and VTAB_K - per-instance callbacks

Each implementation generates a `vtabv` struct. An additional `vtabk` struct may be generated if the container operates with keys and values.

```c
#define VTAB_V CVX_(SNAME, _vtabv)

struct VTAB_V {
    int    (*comp)(V, V);
    V      (*copy)(V);
    void   (*drop)(V);
    size_t (*hash)(V);
    int    (*prio)(V, V);
};
```

This is generated by `CVX_VTAB_DEFINITION(V)` from `core.h`. All fields are nullable. The container struct holds a `struct VTAB_V *vtabv` (or also a `struct VTAB_K *vtabk`) pointer, which itself may also be `NULL`.

**Usage pattern in implementations:**

Both the member value (`vtabv` or `vtabk`) and the associated function must be checked for `NULL`. For example:

```c
// copy
if (_self_->vtabv && _self_->vtabv->copy)
    dest = _self_->vtabv->copy(src);
else
    dest = src;   // direct assignment (shallow)

// drop
if (_self_->vtabv && _self_->vtabv->drop)
    _self_->vtabv->drop(value);
```

---

## Error handling

Every data structure (implementation and iterators) have a flag field, defined in the `cvx_container` type. This flag must be set in every function. If the function's purpose succeeds, it must set the flag to `CVX_FLAG_OK`. Otherwise, it must set it to one of the possible values of `enum cvx_flags`.

Initialization functions that return `cvx_container *` return `NULL` on allocation failure (the caller must check for NULL, not the flag, in this case).

Functions that return `V` or `K` return `(V){ 0 }` or `(K){ 0 }` respectively, in case of an error.

---

## Test structure

### File organisation

Tests are `.h` files in `tests/`. They are included by `tests.c`, which is the single translation unit for the test binary. Each test file:

- Has an include guard
- Includes `"cvxtest.h"` and `"implementations.h"`
- Defines `static void test_<pfx>_<scenario>(struct cvxtest *t)` functions
- Defines a `static int run_<suite_name>_tests(void)` runner that calls `CVXRUN` for each test and returns `CVXSUMMARY`

`tests.c` includes all test headers and registers every runner in a function pointer array.

### Test file naming

| File | Contents |
|---|---|
| `tests/<impl>_tests.h` | Core functional tests for an implementation |
| `tests/<impl>_guard_tests.h` | Wrong-tag guard tests for every function |
| `tests/<impl>_iter_tests.h` | Direct iterator function tests |
| `tests/<impl>_vtabv_tests.h` | Callback invocation count tests |
| `tests/interface/<intf>_<impl>_tests.h` | Tests via an interface |
| `tests/iter/<impl>_iter_<kind>.h` | Tests via an iterator interface |

### Test function naming

```
test_<pfx>_<function>_<scenario>
```

Examples: `test_da_int_push_back`, `test_sll_int_clone_values`, `test_da_vtabv_drop_called_on_clear`.

Guard test names: `test_<pfx>_guard_<function>` (e.g. `test_da_int_guard_clone`).

### Test suite runner naming

```
run_<subject>_tests
```

Examples: `run_dynamic_array_tests`, `run_slinked_list_vtabv_tests`, `run_stack_dynamic_array_tests`.

### Shared test infrastructure

**`tests/implementations.h`** - the single header that instantiates all concrete types used across tests. All test files include this rather than instantiating their own types. New types must be added here.

**`tests/vtabs.h`** - defines shared vtabv callback functions and their invocation counters:

```c
static size_t cvx_counter_vtab_comp = 0;
static size_t cvx_counter_vtab_copy = 0;
static size_t cvx_counter_vtab_drop = 0;

int   int_comp(int a, int b);  // increments cvx_counter_vtab_comp
int   int_copy(int a);         // increments cvx_counter_vtab_copy
void  int_drop(int a);         // increments cvx_counter_vtab_drop

#define CVX_TEST_COUNTER_COMP(t, n)  CVXCHECK(t, n == cvx_counter_vtab_comp)
#define CVX_TEST_COUNTER_COPY(t, n)  CVXCHECK(t, n == cvx_counter_vtab_copy)
#define CVX_TEST_COUNTER_DROP(t, n)  CVXCHECK(t, n == cvx_counter_vtab_drop)
#define CVX_TEST_COUNTER_COMP_RESET() cvx_counter_vtab_comp = 0
#define CVX_TEST_COUNTER_COPY_RESET() cvx_counter_vtab_copy = 0
#define CVX_TEST_COUNTER_DROP_RESET() cvx_counter_vtab_drop = 0
```

vtabv callback tests must reset the relevant counter at the start of each test.

**`tests/cvxtestutils.h`** - small helpers:

```c
#define cvx_col(ds) ((cvx_container *)(&(ds)))   // convert stack-alloc struct to cvx_container *
#define MAKE_INVALID_CONTAINER(name) \
    cvx_container *name = &(cvx_container){ .flag = 0, .tag = 999999 };
```

### Test categories and what to cover

For every new implementation, add tests in this order:

1. **Core tests** - init/new constructors, every mutator and getter, clone, clear, error paths (empty, range, alloc)
2. **Guard tests** - one test per public function that takes `cvx_container *`, using `MAKE_INVALID_CONTAINER`; verify `CVX_FLAG_WRONG_TAG` is set and the error return is correct
3. **Iterator tests** - direct iterator function tests (stack-allocated and heap-allocated constructors, all movement and access functions, wrong-tag guard on each)
4. **vtabv tests** - verify copy/drop counters increment the correct number of times for clone, drop, and clear; verify NULL vtabv paths do not crash

---

## Adding a new implementation - checklist

1. Create `cvx/<name>.h` following the function ordering and naming conventions above.
2. Define `VTAB_V`, the container struct (with `vtabv` field), any node struct, and the iterator struct.
3. Implement all mandatory constructors, destructors, and operations.
4. Add `#ifdef IMPL_*` blocks for each interface/iterator the implementation supports.
5. End with `#undef VTAB_V` and `#include "cvx/undef.h"`.
6. Add the instantiation to `tests/implementations.h`.
7. Create the four test files (core, guard, iterator, vtabv) and register their runners in `tests.c`.
8. Update the tables in `README.md`.

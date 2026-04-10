TODO: iterators

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
  - [File content ordering](#file-content-ordering)
  - [VTAB\_V and VTAB\_K - per-instance callbacks](#vtab_v-and-vtab_k---per-instance-callbacks)
  - [Error handling](#error-handling)
  - [Test structure](#test-structure)
    - [File organisation](#file-organisation)
    - [Test file naming](#test-file-naming)
    - [Test function naming](#test-function-naming)
    - [Test suite runner naming](#test-suite-runner-naming)
    - [Shared test infrastructure](#shared-test-infrastructure)
    - [Test categories and what to cover](#test-categories-and-what-to-cover)
    - [Test ordering](#test-ordering)

---

## Overview

cvx is a header-only, generic C data structure library built on a C preprocessor templating pattern. Each header file is a template: the caller defines a set of macros, includes the header, and gets a fully concrete, type-safe struct and its associated functions. Each `#include` is one independent instantiation.

There are three kinds of header files:

- **Implementations** (`cvx/*.h`) - concrete data structures and iterators (`dynamic_array`, `slinked_list`, etc.)
- **Interfaces** (`cvx/interface/*.h`) - abstract operation sets (`stack`, `queue`, etc.)
- **Iterators** (`cvx/iter/*.h`) - abstract traversal protocols (`forward_iterator`, `random_access_iterator`, etc.)

---

## Implementations

An implementation is instantiated by defining a set of macros and then including a header. Each `#include` produces a new and independent concrete type. For example:

```c
// Check out the Macro Conventions section for more information.
#define V         char *
#define SNAME     str_array
#define PFX       sa
#define TAG       1
#include "cvx/dynamic_array.h"
```

All macros are automatically undefined after the `#include` via an `cvx/undef.h` which is included at the very end, so there is no risk of them anything into subsequent templates.

The implementation type provides a wide range of functionality (even if certain operations are slow), and at least one iterator.
This is a generalist data structure, so there is no well-defined interface. If it could implement a certain operation, then why not.

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
| `cvx_container` pointer | `_col_` |
| The implementing container | `_self_` |
| Original in clone | `_orig_` |
| Copy in clone | `_copy_` |
| Results | `_res_` |
| Value | `_val_` |
| Key | `_key_` |
| New key or value (replace) | `_new_` |
| Old key value (replace return) | `_old_` |
| Nodes | `_node_`, `_curr_`, etc. |
| vtabv argument | `_vtabv_` |
| vtabk argument | `_vtabk_` |
| Iterator | `_iter_` |
| Target (iter constructor) | `_target_` |
| Other parameters and variables | `_steps_`, `_item_`, `_index_`, etc. |

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

## File content ordering

An implementation file can have a multitude of things. To make it easier to navigate, items must be in the following order:

1. `#ifndef` guards for each required macro, with an error message following the template:
  * `#error "path/to/file.h requires <MACRO> to be defined (what it is used for, e.g. #define <MACRO> short_example)`
  * For example: `#error "cvx/dlinked_list.h requires SNAME to be defined (the struct name, e.g. #define SNAME my_list)"`
  * This initial block must be surrounded by a comment containing `clang-format off` and then `clang-format on` to prevent weird line breaks from the long error messages
2. Any necessary includes, from either C's libraries or `cvx/core.h`
3. Internal macro definitions, like `FUNC`, `NODE`, `ITERATOR`, `ITER_TAG`, `VTAB_V`, `VTAB_K`, `ENTRY`, etc.
4. All struct definitions, like `struct SNAME`, `struct ITERATOR`, etc.
5. All function definitions for the data structure, following a specific order:
  * Initializers and destructors come first (but between them there is no specific order)
  * Getters come second (accessing specific properties of the struct)
  * Other operations come last (in no specific order)
6. All iterator function definitions
  * They must also follow a similar logic to the data structure, where
  * Initializers come first, then getters and then the other operations
7. All implementation-detail function definitions
8. Implementation of all functions, following the correct order as they are declared in the previous sections
9. All interface casts that the implementation supports
10. The very last item must be an `#include "cvx/undef.h"`

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
| `tests/<impl>_iter_tests.h` | Direct iterator function tests |
| `tests/<impl>_guard_tests.h` | Wrong-tag guard tests for every function that has a `CVX_CONTAINER_GUARDS` guard |
| `tests/<impl>_vtab_tests.h` | Callback invocation count tests for both `V` and `K` (if applicable) functions |
| `tests/interface/<intf>_<impl>_tests.h` | Tests via an interface |
| `tests/iter/<impl>_iter_<kind>.h` | Tests via an iterator interface |

### Test function naming

```
test_<group>_<pfx>_<function>_<scenario>
```

- `group` - for example, `iter`, `guard`, `alloc`, `vtab` or empty if this is the core functionality test
- `pfx` - the `PFX` macro used to generate the function namespaces
- `function` - the main function being tested
- `scenario` - a specific scenario that is being tested for the function

### Test suite runner naming

```
run_<group>_<subject>_tests
```

### Shared test infrastructure

**`tests/implementations.h`** - the single header that instantiates all concrete types used across tests. All test files include this rather than instantiating their own types. New types must be added here.
**`tests/vtabs.h`** - defines shared vtabv and vtabk callback functions and their invocation counters. vtabv callback tests must reset the relevant counter at the start of each test.
**`tests/alloc.h`** - overrides malloc and calloc for testing allocation error paths.
**`tests/cvxtestutils.h`** - shared utility functions:

### Test categories and what to cover

For every new implementation, add the following tests:

1. **Core tests** - all constructors and destructors, every getter, every operation, error paths that might produce an `enum cvx_flag`; must not contain iterator tests.
2. **Iterator tests** - direct iterator function tests, including all defined iterator-specific functions (like stack-allocated and heap-allocated constructors, all movement and access functions, etc.).
3. **Guard tests** - one test per public function that takes `cvx_container *`, using `MAKE_INVALID_CONTAINER`; verify `CVX_FLAG_WRONG_TAG` is set and the error return is correct; must be done for both the data structure and its iterator.
4. **vtab tests** - verify copy/drop counters increment the correct number of times for clone, drop, and clear; verify NULL vtab paths do not crash; add tests for both `V` and `K` vtabs if the latter is present.

### Test ordering

- Within the same group (or file), like `alloc`, `iter`, `guard`, etc., tests must be ordered according to the declaration order in the implementation file
- This means both the test function body and the test function call must be ordered
- For example, how `_init` comes before `_init_with` and then other functions follow a declaration order
- Tests for the same operation or function, must also be grouped together
- This is to make it easier to find tests, by following the natural order that they appear in the source code

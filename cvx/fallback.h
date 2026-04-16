/// This file defines a bunch of macros only when I'm using clangd LSP.
/// This is enabled by defining CVX_ENABLE_FALLBACK.
///
/// Since I'm writing templating headers, clangd doesn't understand where macros
/// like V and SNAME are defined. Basically, the header files aren't stand-alone
/// and I need to include this file at the top of every templating header file
/// so that I get decent LSP features.
///
/// It is not ideal, but I think it works for now.
#ifndef CVX_FALLBACK_H
#define CVX_FALLBACK_H

#ifdef CVX_ENABLE_FALLBACK

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Container template macros
#ifndef K
#define K int
#endif
#ifndef V
#define V int
#endif
#ifndef SNAME
#define SNAME cvx_fallback
#endif
#ifndef PFX
#define PFX cvx_fb
#endif
#ifndef TAG
#define TAG 99
#endif

// Interface macros
#ifndef INTERFACE
#define INTERFACE cvx_fallback_interface
#endif

// Stack / queue / deque impl macros
#ifndef IMPL_NEW
#define IMPL_NEW 0
#endif
#ifndef IMPL_DROP
#define IMPL_DROP 0
#endif
#ifndef IMPL_CLONE
#define IMPL_CLONE 0
#endif
#ifndef IMPL_PUSH
#define IMPL_PUSH 0
#endif
#ifndef IMPL_POP
#define IMPL_POP 0
#endif
#ifndef IMPL_COUNT
#define IMPL_COUNT 0
#endif
#ifndef IMPL_PEEK
#define IMPL_PEEK 0
#endif
#ifndef IMPL_REPLACE
#define IMPL_REPLACE 0
#endif
#ifndef IMPL_ENQUEUE
#define IMPL_ENQUEUE 0
#endif
#ifndef IMPL_DEQUEUE
#define IMPL_DEQUEUE 0
#endif
#ifndef IMPL_PUSH_FRONT
#define IMPL_PUSH_FRONT 0
#endif
#ifndef IMPL_PUSH_BACK
#define IMPL_PUSH_BACK 0
#endif
#ifndef IMPL_POP_FRONT
#define IMPL_POP_FRONT 0
#endif
#ifndef IMPL_POP_BACK
#define IMPL_POP_BACK 0
#endif
#ifndef IMPL_PEEK_FRONT
#define IMPL_PEEK_FRONT 0
#endif
#ifndef IMPL_PEEK_BACK
#define IMPL_PEEK_BACK 0
#endif

// Iterator impl macros
#ifndef IMPL_START
#define IMPL_START 0
#endif
#ifndef IMPL_END
#define IMPL_END 0
#endif
#ifndef IMPL_AT_START
#define IMPL_AT_START 0
#endif
#ifndef IMPL_AT_END
#define IMPL_AT_END 0
#endif
#ifndef IMPL_TO_START
#define IMPL_TO_START 0
#endif
#ifndef IMPL_TO_END
#define IMPL_TO_END 0
#endif
#ifndef IMPL_NEXT
#define IMPL_NEXT 0
#endif
#ifndef IMPL_PREV
#define IMPL_PREV 0
#endif
#ifndef IMPL_FORWARD
#define IMPL_FORWARD 0
#endif
#ifndef IMPL_BACKWARD
#define IMPL_BACKWARD 0
#endif
#ifndef IMPL_GO_TO
#define IMPL_GO_TO 0
#endif
#ifndef IMPL_VALUE
#define IMPL_VALUE 0
#endif
#ifndef IMPL_INDEX
#define IMPL_INDEX 0
#endif

#endif // CVX_ENABLE_FALLBACK

#endif // CVX_FALLBACK_H

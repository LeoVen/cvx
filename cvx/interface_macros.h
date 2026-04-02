#ifndef CVX_HELPER_MACROS_H
#define CVX_HELPER_MACROS_H

#define cvx_new(s) (s)->vtable->new()
#define cvx_clone(s) (s)->vtable->clone(s)
#define cvx_push(s, v) (s)->vtable->push((s)->instance, v)
#define cvx_pop(s, vout) (s)->vtable->pop((s)->instance, vout)
#define cvx_count(s) (s)->vtable->count((s)->instance)
#define cvx_drop(s) (s)->vtable->drop((s)->instance)
#define cvx_peek(s) (s)->vtable->peek((s)->instance)
#define cvx_replace(s, new, out) (s)->vtable->replace((s)->instance, new, out)
#define cvx_flag(s) ((cvx_container *)(s)->instance)->flag

#endif /* CVX_HELPER_MACROS_H */

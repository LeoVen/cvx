#ifndef CVX_HELPER_MACROS_H
#define CVX_HELPER_MACROS_H

#define cvx_at_end(ds) (ds)->vtable->at_end((ds)->instance)
#define cvx_at_start(ds) (ds)->vtable->at_start((ds)->instance)
#define cvx_backward(ds, b) (ds)->vtable->backward((ds)->instance, b)
#define cvx_clone(ds) (ds)->vtable->clone((ds)->instance)
#define cvx_count(ds) (ds)->vtable->count((ds)->instance)
#define cvx_dequeue(ds) (ds)->vtable->dequeue((ds)->instance)
#define cvx_drop(ds) (ds)->vtable->drop((ds)->instance)
#define cvx_end(ds) (ds)->vtable->end((ds)->instance)
#define cvx_enqueue(ds, item) (ds)->vtable->enqueue((ds)->instance, item)
#define cvx_flag(ds) ((cvx_container *)(ds)->instance)->flag
#define cvx_forward(ds, f) (ds)->vtable->forward((ds)->instance, f)
#define cvx_index(ds) (ds)->vtable->index((ds)->instance)
#define cvx_next(ds) (ds)->vtable->next((ds)->instance)
#define cvx_new(ds) (ds)->vtable->new()
#define cvx_peek(ds) (ds)->vtable->peek((ds)->instance)
#define cvx_peek_back(ds) (ds)->vtable->peek_back((ds)->instance)
#define cvx_peek_front(ds) (ds)->vtable->peek_front((ds)->instance)
#define cvx_pop(ds) (ds)->vtable->pop((ds)->instance)
#define cvx_pop_back(ds) (ds)->vtable->pop_back((ds)->instance)
#define cvx_pop_front(ds) (ds)->vtable->pop_front((ds)->instance)
#define cvx_prev(ds) (ds)->vtable->prev((ds)->instance)
#define cvx_push(ds, v) (ds)->vtable->push((ds)->instance, v)
#define cvx_push_back(ds, v) (ds)->vtable->push_back((ds)->instance, v)
#define cvx_push_front(ds, v) (ds)->vtable->push_front((ds)->instance, v)
#define cvx_replace(ds, new) (ds)->vtable->replace((ds)->instance, new)
#define cvx_start(ds) (ds)->vtable->start((ds)->instance)
#define cvx_to_start(ds) (ds)->vtable->to_start((ds)->instance)
#define cvx_to_end(ds) (ds)->vtable->to_end((ds)->instance)
#define cvx_value(ds) (ds)->vtable->value((ds)->instance)

#endif /* CVX_HELPER_MACROS_H */

#ifndef CVX2_NAMES_H
#define CVX2_NAMES_H

#ifdef CVX_NAMES_PASCALCASE

#define NAME_vtabv VtabV
#define NAME_vtabk VtabK
#define NAME_entry Entry
#define NAME_node Node

#define NAME_init Init
#define NAME_drop Drop
#define NAME_clone Clone
#define NAME_count Count
#define NAME_capacity Capacity
#define NAME_empty Empty
#define NAME_full Full
#define NAME_front Front
#define NAME_back Back
#define NAME_get Get
#define NAME_push_front PushFront
#define NAME_push_at PushAt
#define NAME_push_back PushBack
#define NAME_pop_front PopFront
#define NAME_pop_at PopAt
#define NAME_pop_back PopBack
#define NAME_replace_front ReplaceFront
#define NAME_replace_back ReplaceBack
#define NAME_swap Swap
#define NAME_compare Compare
#define NAME_sort Sort
#define NAME_load Load
#define NAME_insert Insert
#define NAME_update Update
#define NAME_remove Remove
#define NAME_get_ref GetRef
#define NAME_contains Contains

#define NAME__assert_capacity __AssertCapacity
#define NAME__assert_buffer __AssertBuffer
#define NAME__init_buffer __InitBuffer
#define NAME__drop_buffer __DropBuffer
#define NAME__clone_buffer __CloneBuffer
#define NAME__get_ref __GetRef
#define NAME__insert_unchecked __InsertUnchecked
#define NAME__resize __Resize
#define NAME__remove __Remove
#define NAME__find_entry __FindEntry
#define NAME__find_node __FindNode
#define NAME__next_prime __NextPrime
#define NAME__primes __Primes
#define NAME__primes_count __PrimesCount

#elif defined(CVX_NAMES_CAMELCASE)

#define NAME_vtabv vtabV
#define NAME_vtabk vtabK
#define NAME_entry entry
#define NAME_node node

#define NAME_init init
#define NAME_drop drop
#define NAME_clone clone
#define NAME_count count
#define NAME_capacity capacity
#define NAME_empty empty
#define NAME_full full
#define NAME_front front
#define NAME_back back
#define NAME_get get
#define NAME_push_front pushFront
#define NAME_push_at pushAt
#define NAME_push_back pushBack
#define NAME_pop_front popFront
#define NAME_pop_at popAt
#define NAME_pop_back popBack
#define NAME_replace_front replaceFront
#define NAME_replace_back replaceBack
#define NAME_swap swap
#define NAME_compare compare
#define NAME_sort sort
#define NAME_load load
#define NAME_insert insert
#define NAME_update update
#define NAME_remove remove
#define NAME_get_ref getRef
#define NAME_contains contains

#define NAME__assert_capacity __assertCapacity
#define NAME__assert_buffer __assertBuffer
#define NAME__init_buffer __initBuffer
#define NAME__drop_buffer __dropBuffer
#define NAME__clone_buffer __cloneBuffer
#define NAME__get_ref __getRef
#define NAME__insert_unchecked __insertUnchecked
#define NAME__resize __resize
#define NAME__remove __remove
#define NAME__find_entry __findEntry
#define NAME__find_node __findNode
#define NAME__next_prime __nextPrime
#define NAME__primes __primes
#define NAME__primes_count __primesCount

#else

#define NAME_vtabv _vtabv
#define NAME_vtabk _vtabk
#define NAME_entry _entry
#define NAME_node _node

#define NAME_init _init
#define NAME_drop _drop
#define NAME_clone _clone
#define NAME_count _count
#define NAME_capacity _capacity
#define NAME_empty _empty
#define NAME_full _full
#define NAME_front _front
#define NAME_back _back
#define NAME_get _get
#define NAME_push_front _push_front
#define NAME_push_at _push_at
#define NAME_push_back _push_back
#define NAME_pop_front _pop_front
#define NAME_pop_at _pop_at
#define NAME_pop_back _pop_back
#define NAME_replace_front _replace_front
#define NAME_replace_back _replace_back
#define NAME_swap _swap
#define NAME_compare _compare
#define NAME_sort _sort
#define NAME_load _load
#define NAME_insert _insert
#define NAME_update _update
#define NAME_remove _remove
#define NAME_get_ref _get_ref
#define NAME_contains _contains

#define NAME__assert_capacity __assert_capacity
#define NAME__assert_buffer __assert_buffer
#define NAME__init_buffer __init_buffer
#define NAME__drop_buffer __drop_buffer
#define NAME__clone_buffer __clone_buffer
#define NAME__get_ref __get_ref
#define NAME__insert_unchecked __insert_unchecked
#define NAME__resize __resize
#define NAME__remove __remove
#define NAME__find_entry __find_entry
#define NAME__find_node __find_node
#define NAME__next_prime __next_prime
#define NAME__primes __primes
#define NAME__primes_count __primes_count

#endif

#endif /* CVX2_NAMES_H */

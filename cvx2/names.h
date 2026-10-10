#ifndef CVX2_NAMES_H
#define CVX2_NAMES_H

#ifdef CVX_NAMES_PASCALCASE

#define NAME_entry Entry
#define NAME_node Node
#define NAME_vtabk VtabK
#define NAME_vtabv VtabV

#define NAME_append Append
#define NAME_back Back
#define NAME_capacity Capacity
#define NAME_clone Clone
#define NAME_compare Compare
#define NAME_contains Contains
#define NAME_count Count
#define NAME_drop Drop
#define NAME_empty Empty
#define NAME_front Front
#define NAME_full Full
#define NAME_get Get
#define NAME_get_ref GetRef
#define NAME_index_of IndexOf
#define NAME_init Init
#define NAME_insert Insert
#define NAME_load Load
#define NAME_pop Pop
#define NAME_prepend Prepend
#define NAME_push Push
#define NAME_remove Remove
#define NAME_replace Replace
#define NAME_resize Resize
#define NAME_seq_push SeqPush
#define NAME_shrink Shrink
#define NAME_sort Sort
#define NAME_sublist Sublist
#define NAME_swap Swap
#define NAME_truncate Truncate
#define NAME_update Update

#define NAME__assert_buffer __AssertBuffer
#define NAME__assert_capacity __AssertCapacity
#define NAME__assert_capacity_for __AssertCapacityFor
#define NAME__clone_buffer __CloneBuffer
#define NAME__drop_buffer __DropBuffer
#define NAME__find_entry __FindEntry
#define NAME__find_node __FindNode
#define NAME__get_ref __GetRef
#define NAME__init_buffer __InitBuffer
#define NAME__insert_unchecked __InsertUnchecked
#define NAME__next_prime __NextPrime
#define NAME__primes __Primes
#define NAME__primes_count __PrimesCount
#define NAME__remove __Remove
#define NAME__resize __Resize

#elif defined(CVX_NAMES_CAMELCASE)

#define NAME_entry entry
#define NAME_node node
#define NAME_vtabk vtabK
#define NAME_vtabv vtabV

#define NAME_append append
#define NAME_back back
#define NAME_capacity capacity
#define NAME_clone clone
#define NAME_compare compare
#define NAME_contains contains
#define NAME_count count
#define NAME_drop drop
#define NAME_empty empty
#define NAME_front front
#define NAME_full full
#define NAME_get get
#define NAME_get_ref getRef
#define NAME_index_of indexOf
#define NAME_init init
#define NAME_insert insert
#define NAME_load load
#define NAME_pop pop
#define NAME_prepend prepend
#define NAME_push push
#define NAME_remove remove
#define NAME_replace replace
#define NAME_resize resize
#define NAME_seq_push seqPush
#define NAME_shrink shrink
#define NAME_sort sort
#define NAME_sublist sublist
#define NAME_swap swap
#define NAME_truncate truncate
#define NAME_update update

#define NAME__assert_buffer __assertBuffer
#define NAME__assert_capacity __assertCapacity
#define NAME__assert_capacity_for __assertCapacityFor
#define NAME__clone_buffer __cloneBuffer
#define NAME__drop_buffer __dropBuffer
#define NAME__find_entry __findEntry
#define NAME__find_node __findNode
#define NAME__get_ref __getRef
#define NAME__init_buffer __initBuffer
#define NAME__insert_unchecked __insertUnchecked
#define NAME__next_prime __nextPrime
#define NAME__primes __primes
#define NAME__primes_count __primesCount
#define NAME__remove __remove
#define NAME__resize __resize

#else

#define NAME_entry _entry
#define NAME_node _node
#define NAME_vtabk _vtabk
#define NAME_vtabv _vtabv

#define NAME_append _append
#define NAME_back _back
#define NAME_capacity _capacity
#define NAME_clone _clone
#define NAME_compare _compare
#define NAME_contains _contains
#define NAME_count _count
#define NAME_drop _drop
#define NAME_empty _empty
#define NAME_front _front
#define NAME_full _full
#define NAME_get _get
#define NAME_get_ref _get_ref
#define NAME_index_of _index_of
#define NAME_init _init
#define NAME_insert _insert
#define NAME_load _load
#define NAME_pop _pop
#define NAME_prepend _prepend
#define NAME_push _push
#define NAME_remove _remove
#define NAME_replace _replace
#define NAME_resize _resize
#define NAME_seq_push _seq_push
#define NAME_shrink _shrink
#define NAME_sort _sort
#define NAME_sublist _sublist
#define NAME_swap _swap
#define NAME_truncate _truncate
#define NAME_update _update

#define NAME__assert_buffer __assert_buffer
#define NAME__assert_capacity __assert_capacity
#define NAME__assert_capacity_for __assert_capacity_for
#define NAME__clone_buffer __clone_buffer
#define NAME__drop_buffer __drop_buffer
#define NAME__find_entry __find_entry
#define NAME__find_node __find_node
#define NAME__get_ref __get_ref
#define NAME__init_buffer __init_buffer
#define NAME__insert_unchecked __insert_unchecked
#define NAME__next_prime __next_prime
#define NAME__primes __primes
#define NAME__primes_count __primes_count
#define NAME__remove __remove
#define NAME__resize __resize

#endif

#endif

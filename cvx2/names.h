#ifndef CVX2_NAMES_H
#define CVX2_NAMES_H

#ifdef CVX_STRUCT_NAMES_PASCALCASE

#define NAME_entry Entry
#define NAME_node Node
#define NAME_vtabk VtabK
#define NAME_vtabv VtabV
#define NAME_internal Internal

#elif defined(CVX_STRUCT_NAMES_CAMELCASE)

#define NAME_entry entry
#define NAME_node node
#define NAME_vtabk vtabK
#define NAME_vtabv vtabV
#define NAME_internal internal

#else

#define NAME_entry _entry
#define NAME_node _node
#define NAME_vtabk _vtabk
#define NAME_vtabv _vtabv
#define NAME_internal _internal

#endif

#ifdef CVX_FUNC_NAMES_PASCALCASE

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

#elif defined(CVX_FUNC_NAMES_CAMELCASE)

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

#else

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

#endif

#endif

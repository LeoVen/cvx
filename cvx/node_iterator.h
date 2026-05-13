/// node_iterator.h
///
/// Status
///
///   [ ] concept
///   [ ] v1
///   [ ] tests
///   [ ] refine
///   [ ] stabilize
///
/// A node iterator can operate in any node-based data structure.
///
/// Options
///
///   LINEAR - Operates on a data structure with a "next" pointer
///   BIDI_LINEAR - Like LINEAR but with a "prev" pointer
///   TREE - Iterator on a tree-like structure
///
/// Expected usage
///
///   for (struct iterator it = init_iter(&arr); !iter_at_end(&it); iter_next(&it))
///   {
///       char *key = iter_key(&it);
///       int value = iter_value(&it);
///       ...
///   }
///
/// Parameters:
/// - NULLCHECKS: if enabled, items are checked against NULL and skipped if so
/// - CIRCULAR: if enabled, the pointer wraps around the buffer's capacity

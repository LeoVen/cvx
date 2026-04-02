#ifndef CVX_CORE_H
#define CVX_CORE_H

#include <stdbool.h>

#define CVX__(A, B) A##B
#define CVX_(A, B) CVX__(A, B)

#define CVX_CONTAINER_GUARDS(TAG, _col_, error_value) \
    if (_col_->tag != TAG) \
    { \
        _col_->flag = CVX_FLAG_WRONG_TAG; \
        return error_value; \
    }

#endif /* CVX_CORE_H */

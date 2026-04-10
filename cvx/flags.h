#ifndef CVX_FLAGS_H
#define CVX_FLAGS_H

/**
 * enum cvx_flags
 *
 * Defines common error codes used by the entire library.
 */
// clang-format off
enum cvx_flags
{
    CVX_FLAG_OK        =  0, // No errors
    CVX_FLAG_WRONG_TAG =  1, // Tags are checked so you don't pass the wrong struct to a function
    CVX_FLAG_ALLOC     =  2, // Allocation failed
    CVX_FLAG_EMPTY     =  3, // Operation can not proceed because the container is empty
    CVX_FLAG_FULL      =  4, // When a container that doesn't resize is full
    CVX_FLAG_RANGE     =  5, // Index out of range
    CVX_FLAG_NOT_FOUND =  6, // Key or value not found
    CVX_FLAG_INVALID   =  7, // Invalid argument or operation
    CVX_FLAG_DUPLICATE =  8, // Duplicate key or value
    CVX_FLAG_ERROR     =  9, // Generic error, or unknown error
    CVX_FLAG_VTAB      = 10, // Required vtab function was not provided
};
// clang-format on

/**
 * cvx_flags_str
 *
 * Maps the error codes to their character representation.
 */
// clang-format off
static const char *cvx_flags_str[] = {
    "OK",          // 0
    "WRONG_TAG",   // 1
    "ALLOC",       // 2
    "EMPTY",       // 3
    "FULL",        // 4
    "RANGE",       // 5
    "NOT_FOUND",   // 6
    "INVALID",     // 7
    "DUPLICATE",   // 8
    "ERROR",       // 9
    "VTAB",        // 10
};
// clang-format on

/**
 * cvx_flags_description
 *
 * Maps the error codes to their description strings.
 */
// clang-format off
static const char *cvx_flags_description[] = {
    "No error",                                      // 0
    "Wrong container type for this function",        // 1
    "Allocation failed",                             // 2
    "The container is empty when it should not be",  // 3
    "Container is full",                             // 4
    "Index out of range",                            // 5
    "Key or value not found",                        // 6
    "Invalid argument or operation",                 // 7
    "Duplicate key or value",                        // 8
    "Generic or unknown error",                      // 9
    "Required VTAB function missing",                // 10
};
// clang-format on

#endif /* CVX_FLAGS_H */

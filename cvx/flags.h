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
};
// clang-format on

/**
 * cvx_flags_str
 *
 * Maps the error codes to their character representation.
 */
// clang-format off
static const char *cvx_flags_str[12] = {
    "OK",
    "ALLOC",
    "EMPTY",
    "NOT_FOUND",
    "INVALID",
    "RANGE",
    "DUPLICATE",
    "ERROR",
    "THREAD",
    "MUTEX",
    "FULL",
    "FTABLE"
};
// clang-format on

/**
 * cvx_flags_description
 *
 * Maps the error codes to their character representation.
 */
// clang-format off
static const char *cvx_flags_description[12] = {
    "No Error",
    "Allocation failed",
    "The container is empty when it should not",
    "Key or value not found",
    "Invalid argument or operation",
    "Index out of range",
    "Duplicate key or value",
    "Generic error",
    "Generic error for threads",
    "Generic error for mutexes",
    "Collection is full",
    "Required ftable function is missing",
};
// clang-format on

#endif /* CVX_FLAGS_H */

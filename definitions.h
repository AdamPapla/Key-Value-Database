/**
 * @brief Header file containing important definitions of max key and value sizes and defn of kv_pair struct
 */
#ifndef UTILITY_H_
#define UTILITY_H_

#define MAX_KEY_SIZE 256
#define MAX_VALUE_SIZE 1024

#include <time.h>

typedef struct kv_pair{
    size_t keySize;
    size_t valueSize;
    time_t firstSet;
    time_t lastSet;
} Pair;

#endif

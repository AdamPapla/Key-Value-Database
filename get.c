/**
 * @brief   File containing functions to get KV pair/timestamp given the key
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "definitions.h"
#include "index.h"
/**
 * @brief Small function to print time in the required format to stdout
 * @param[in]   time    Time object containing seconds since 1/1/1970
*/
void printTime(time_t time){
    char buf[50];
    struct tm *ts = localtime(&time);
    strftime(buf, 50, "%Y-%m-%d %H:%M:%S", ts);
    printf("%s", buf);
}
/**
 * @brief Function that uses the index and file offsets in the index.bin file to quickly retrieve KV pairs.
 * @param[in]   data    Pointer to file containing data
 * @param[in]   index   Pointer to file holding index
 * @param[in]   mode    Whether get function should return KV or timestamp. mode == 0 => KV pair, mode == 1 => timestamp
 * @return  Returns 1 if key is found, 0 otherwise.
*/
int get(FILE* data, FILE* index, char* key, int mode){
    // First get index of key, if it exists
    long int startCount, endCount;
    int exists = getDataIndex(index, key, &startCount, &endCount);
    if (exists==0){
        printf("Key not found\n");
        return -1;
    }
    // Seek the starting offset given by the index file
    fseek(data, startCount, SEEK_SET);
    size_t readKeySize = 0; size_t readValueSize = 0;
    time_t firstSet, lastSet;

    // While within bounds given by index file, search for the key
    while(fread(&readKeySize, sizeof(size_t), 1, data) != 0 && ftell(data) <= endCount){
        fread(&readValueSize, sizeof(size_t), 1, data);
        // Use dynamically allocated memory due to variable array length
        char* readKey = malloc(readKeySize);
        char* readValue = malloc(readValueSize);
        fread(readKey, 1, readKeySize, data);
        fread(readValue, 1, readValueSize, data);
        readKey[readKeySize-1] = '\0'; readValue[readValueSize-1] = '\0';
        fread(&firstSet, sizeof(time_t), 1, data);
        fread(&lastSet, sizeof(time_t), 1, data);
        // If keys match, return either the KV pair or the timestamp
        if (strcmp(key, readKey) == 0){
            if (mode == 0)
                printf("Key: %s, value %s\n", key, readValue);
            else if (mode == 1){
                printf("Time first set:\t");
                printTime(firstSet);
                printf("\nTime last set:\t");
                printTime(lastSet);
                printf("\n");
            }
            return 1;
        }
        free(readKey);
        free(readValue);
    }
    printf("Key not found\n");
    return 0;
}

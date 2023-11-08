#ifndef INDEX_C_
#define INDEX_C_
#include <stdio.h>
int compareString(char* a, char* b);
void writeEntry(FILE* tempIndex, char* keyStart, size_t dataCount);
int addIndexLine(FILE* index, char *key, long int pairSize);
size_t getDataIndex(FILE* index, char *key, long int* startCount, long int* endCount);
#endif
#ifndef SET_H_
#define SET_H_


int writePair(FILE* data, char* key, char* value, Pair* kv);
int addToData(FILE* data, char* key, char* value, Pair* entry, long int* difference, int mode);
int set(FILE* data, FILE* index, char* key, char* value, Pair* entry, int mode);

#endif

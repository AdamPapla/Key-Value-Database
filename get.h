#ifndef GET_H_
#define GET_H_

#include <stdlib.h>
#include <stdio.h>

void printTime(time_t time);
int get(FILE* data, FILE* index, char* key, int mode);

#endif

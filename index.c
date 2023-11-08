/**
 * @brief   Function definitions for creating an ordered index of key starts, and their accompanying file offsets in the data.bin file
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * @brief   Compares the first two letters of two entered strings. Return is based on which string comes first when 
 *          ordered alphabetically
 * @param[in]   a   Pointer to the beginning of the first string
 * @param[in]   b   Pointer to the beginning of the second string
 * @return  Returns 0 if start of strings are equal, 1 if the first string is smaller and 2 if the first string is larger
*/
int compareString(char* a, char* b){
    char aStart[3]; char bStart[3];
    strncpy(aStart, a, 2);
    strncpy(bStart, b, 2);
    aStart[2] = '\0'; bStart[2] = '\0';
    if (strcmp(aStart, bStart) == 0)
        return 0;
    else if (a[0]<b[0])
        return 1;
    else if (a[0] == b[0] && a[1] < b[1] && strlen(a) >= 2 && strlen(b) >= 2)
        return 1;
    return 2;
}
/**
 * @brief Small function to write entries to index.bin
 * @param[in]   tempIndex   Pointer to the new index file that is being written to
 * @param[in]   keyStart    Pointer to char array holding the null terminated start of the key.
 * @param[in]   dataCount   Number of bytes written to the file after this entry is written
*/
void writeEntry(FILE* tempIndex, char* keyStart, size_t dataCount){
    if (fwrite(keyStart, 1, 2, tempIndex) != 2) 
        fprintf(stderr, "Error writing key start %s to index file\n", keyStart);
    if (fwrite(&dataCount, sizeof(size_t), 1, tempIndex) != 1) 
        fprintf(stderr, "Error writing dataCount for key start %s to index file\n", keyStart);
}

/**
 * @brief   This function updates the index.bin file when "set" or "del" commands are called. It handles a
 *          number of different cases:
 *              -   set: New key index (i.e. Key with unique two letter start): Creates new entry in index.bin with
 *                  the index and the file offset at which the key ends. 
 *              -   set: Existing key index: Updates the offset of the existing index to account for another KV pair 
 *                  being stored at this index.
 *              -   del: Non existant key: Prints "Key not found"
 *              -   del: Index corresponding to >1 KV pair: Updates the offset to remove element corresponding to entered key
 *              -   del: Index corresponding to 1 KV pair: Deletes this index record from index.bin
 * @param[in]   index       Pointer to index file
 * @param[in]   key         String containing key
 * @param[in]   pairSize    Contains one of the following (measures in bytes) depending on context:
 *                              - Size of new KV pair.
 *                              - Difference in size between new and old KV pairs (they have same key)
 *                              - Size of deleted entry (will be negative)
*/
int addIndexLine(FILE* index, char *key, long int pairSize){
    rewind(index);
    FILE* tempIndex = fopen("tempIndex.bin", "w");
    char ind[3];
    char keyStart[3];
    strncpy(keyStart, key, 2);
    keyStart[2] = '\0';
    int found = 0;
    size_t dataCount = 0;
    size_t prevDataCount = 0;
    // If file is new or wrong format encountered, add as first line
    if (fread(ind, 1, 2, index) == 0){
        dataCount += pairSize;
        writeEntry(tempIndex, keyStart, dataCount);
    }
    else{
        rewind(index);
        while(fread(ind, 1, 2, index) != 0){
            ind[2] = '\0';
            fread(&dataCount, sizeof(size_t), 1, index);
            // If correct posiiton already found, increase offset and continue to writing
            if (found != 0) dataCount += pairSize;
            // Else, check if this is the correct position
            else {
                int result = compareString(keyStart, ind);
                // If strings are equal, increase the count for the next string
                if (result == 0){
                    dataCount += pairSize;
                    found = 1;
                    if (prevDataCount == dataCount){
                        // In this case, the key has no bytes associated with it in data file i.e. it is deleted
                        continue;
                    }
                }
                // If key is less, must update this data count to be prevDataCount + pairSize and update next dataCount 
                else if (result == 1){
                    size_t thisCount = prevDataCount + pairSize;    // Update the new entry's count
                    dataCount = thisCount + (dataCount - prevDataCount);    // Update next 
                    writeEntry(tempIndex, keyStart, thisCount);
                    found = 1;
                }
                prevDataCount = dataCount;
            }
            // Now write back to file
            writeEntry(tempIndex, ind, dataCount);
        }
        // If adding to end of file, must update dataCount accordingly
        if (found == 0){
            dataCount += pairSize;
            writeEntry(tempIndex, keyStart, dataCount);
        }
    }
//    fclose(index);
    fclose(tempIndex);
    remove("index.bin");
    rename("tempIndex.bin", "index.bin");
    index = fopen("index.bin", "r");
    return 0;
}

/**
 * @brief Function to obtain a file offset range that contains a particular key. This can be used to rapidly obtain
 *        keys in the data file. 
 * @param[in]   index   Pointer to index.bin file.
 * @param[in]   key     Pointer to string containing key.
 * @param[out]  startCount  On return, holds the start file offset in bytes, where the key is located within [startCount, endCount]
 * @param[out]  endCount    On return, holds the end file offset in bytes.
*/
size_t getDataIndex(FILE* index, char *key, long int* startCount, long int* endCount){
    rewind(index);
    char ind[3];
    char keyStart[3];
    strncpy(keyStart, key, 2);
    keyStart[2] = '\0';
    // Initialise to 0 in case file is empty
    *startCount = 0;
    *endCount = 0;
    size_t dataCount = 0;
    // Read through to find the correct key start (if there is one) and return it's accompanying file offset range
    while(fread(ind, 1, 2, index) != 0){
        ind[2] = '\0';
        int result = compareString(keyStart, ind);
        fread(&dataCount, sizeof(size_t), 1, index);
        if (result == 0){
            *endCount = dataCount;
            return 1;
        }
        else if (result == 1){
            *endCount = *startCount;
            return 0;
        }
        *startCount = dataCount;
    }
    *endCount = *startCount;
    rewind(index);
    return 0;
}

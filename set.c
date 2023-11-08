/**
 * @brief   File containing function definitions for setting/deleting lines to/from the database
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "index.h"

/**
 * @brief   Simple function to write a KV entry to file
 * @param[in]   data    Pointer to data file
 * @param[in]   key     String containing key
 * @param[in]   value   String containing value
 * @param[in]   kv      Pair object containing sizes of key and value as well as time first set and time last set
*/
int writePair(FILE* data, char* key, char* value, Pair* kv){
    if(fwrite(&(kv -> keySize), sizeof(size_t), 1, data) != 1) perror("ERROR: Failed writing key size\n");
    if(fwrite(&(kv -> valueSize), sizeof(size_t), 1, data) != 1) perror("ERROR: Failed writing value size\n");
    fwrite(key, 1, (kv -> keySize), data);
    fwrite(value, 1, (kv -> valueSize) , data);
    if(fwrite(&(kv -> firstSet), sizeof(time_t), 1, data) != 1) perror("ERROR: Failed writing firstSet\n");
    if(fwrite(&(kv -> lastSet), sizeof(time_t), 1, data) != 1) perror("ERROR: Failed writing lastSet\n");
    return 0;
}

/**
 * @brief   Function to write a new entry into the data file in the correct alphabetical position.
 * @param[in]   data    Pointer to data file
 * @param[in]   key     String containing key to be added
 * @param[in]   value   String containing value to be added
 * @param[in]   entry   Pair object containing sizes of key and value and times key was first set and last set
 * @param[in]   mode    Determines if a key is being added with set or deleted with del
 * @param[out]  difference  Contains, if the key is new, the size of the entry in bytes. If key exists and is being updated, 
 *                          contains the difference in size between the old and new value. If key is being deleted, contains
 *                          the size of the deleted chunk
*/
int addToData(FILE* data, char* key, char* value, Pair* entry, long int* difference, int mode){
    // Check if this is first value to be added
    rewind(data);
    // Open temp file
    FILE* tempData = fopen("tempData.bin", "w") ;
    if(tempData == NULL) printf("Error opening tempData.bin\n");
    Pair* read = malloc(sizeof(Pair));
    char* readKey = malloc(MAX_KEY_SIZE);
    char* readValue = malloc(MAX_VALUE_SIZE); 
    // Pair struct to hold read in entries
    if (fread(&(read -> keySize), sizeof(size_t), 1, data) == 0){
        writePair(tempData, key, value, entry);
    }
    else{
        rewind(data);
        int updated = 0;
        while(fread(&(read -> keySize), sizeof(size_t), 1, data) == 1){
            // Read in current KV entry from data file
            fread(&(read -> valueSize), sizeof(size_t), 1, data);
            readKey = realloc(readKey, read -> keySize);
            readValue = realloc(readValue, read -> valueSize);
            fread(readKey, 1, (read -> keySize), data);
            fread(readValue, 1, (read -> valueSize), data);
            readKey[(read -> keySize)-1] = '\0'; readValue[(read -> valueSize)-1] = '\0';
            if (fread(&(read -> firstSet), sizeof(time_t), 1, data) == 0) printf("firstSet not read in correctly\n");
            if (fread(&(read -> lastSet), sizeof(time_t), 1, data) == 0) printf("lastSet not read in correctly\n");

            // If the key already exists i.e. readKey == key, update and skip writing
            if (strcmp(key, readKey) == 0){
                if (mode == 0){
                    entry -> firstSet = read -> firstSet;   // Update first write
                    // Difference in size of new key value pair
                    *difference = ((entry -> keySize) + (entry -> valueSize)) - ((read -> keySize) + (read -> valueSize));  
                    writePair(tempData, key, value, entry);
                }
                else if (mode == 1){
                    // If deleting key, set difference to be minus the size of the deleted entry
                    *difference = -((read -> keySize) + (read -> valueSize) + 2*sizeof(size_t) + 2*sizeof(time_t));
                }
                updated = 1;
                continue;   // Replace line - don't write current line
            }
            // If the key didn't exist (we've passed where it should be in file), no updates required and don't skip writing
            else if (compareString(key, readKey) == 1 && updated == 0 && mode == 0){
                writePair(tempData, key, value, entry); // Don't replace line
                updated = 1;
            }
            // Write read entry to new temp file
            writePair(tempData, readKey, readValue, read);
        }
        // If not updated by the end of the file, add the entry to the end of file
        if (updated == 0 && mode == 0){
            writePair(tempData, key, value, entry);
            
        }
        else if (updated == 0 && mode == 1){
            printf("Key not found\n");
        }
    }
    // Free dynamically allocated memory
    free(readValue);
    free(readKey);
    free(read);
    // Now close files, delete old file and rename new file
    fclose(tempData);
    remove("data.bin");
    rename("tempData.bin", "data.bin");
    data = fopen("data.bin", "r");

    return 0;
}

/**
 * @brief Function to set a new key, update an old key or delete a key from the database. Handles updates to both index and data files
 * @param[in]   data    Pointer to file containing data
 * @param[in]   index   Pointer to file containing index
 * @param[in]   key     String containing key to be added/updated/deleted
 * @param[in]   value   If key to be added/updated, contains corresponding value. If key is to be deleted, contains "null"
 * @param[in]   entry   Holds other relevant variables; key size, value size and times key was first and last set
 * @param[in]   mode    Determines if key is to be added/updated (mode = 0) or deleted (mode = 1)
*/
int set(FILE* data, FILE* index, char* key, char* value, Pair* entry, int mode){
    size_t keySize = entry -> keySize;  // Extract values from struct
    size_t valueSize = entry -> valueSize;
    long int difference = 2*sizeof(size_t) + 2*sizeof(time_t) + keySize + valueSize;    // Initialise difference to be size of new KV entry
    addToData(data, key, value, entry, &difference, mode);  // Insert this line in data file and update difference var
    addIndexLine(index, key, difference);   // Update index.bin to account for this new/updated/deleted key
    return 0;
}




/**
 * @brief   Main function for implementing IO for the database.
 *          Database is a binary file, with an index file to make getting values faster.
 *          Binary file is quicker to write and read to. Data is stored dynamically and contiguously so memory footprint is small.
 *          All values are stored in alphabetical order in both index.bin and data.bin. This implementation is scalable. If database 
 *          will be large, can extend implementation to have multiple data files holding different alphabetical ranges. 
 *          These will be pointed to by the index file. 
 * @date    24-10-2023
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <sys/file.h>
#include <semaphore.h>
#include "definitions.h"
#include "index.h"
#include "set.h"
#include "get.h"

int main(int argc, char* argv[]){
    sem_t mutex;
    if (sem_init(&mutex, 0, 1) != 0) {
        perror("sem_init");
        return -1;
    }
    // Open files if they exist, else create them 
    FILE* index = fopen("index.bin", "r");
    if (index == NULL) index = fopen("index.bin", "w+");
    FILE* data = fopen("data.bin", "r");
    if (data == NULL) data = fopen("data.bin", "w+");
    
    if (index == NULL){
        perror("Error opening index.bin\n");
        return -1;
    }
    if (data == NULL){
        perror("Error opening data.bin\n");
        return -1;
    }
    char command[10];
    strncpy(command, argv[1], 10);

    if (strcmp(command, "set") == 0){
        if (argc != 4){
            printf("Incorrect numer of arguments entered\nUsage: ./test set key value\n");
            fclose(data); fclose(index);
            return 0;
        }
        // Begin by initialising a Pair struct
        Pair* entry = malloc(sizeof(Pair));
        entry -> keySize = strlen(argv[2]) + 1;
        if (entry -> keySize > MAX_KEY_SIZE){
            fprintf(stderr, "Entered key is too long. To adjust maximum key size, please edit #define in utility.h\n");
            fclose(data); fclose(index);
            return -1;
        }
        entry -> valueSize = strlen(argv[3]) + 1;
        if (entry -> valueSize > MAX_VALUE_SIZE){
            fprintf(stderr, "Entered value is too long. To adjust maximum value size, please edit #define in utility.h\n");
            fclose(data); fclose(index);
            return -1;
        }
        time(&(entry -> firstSet));
        time(&(entry -> lastSet));

        // Use semaphores to ensure exclusive write access 
        sem_wait(&mutex);
        set(data, index, argv[2], argv[3], entry, 0);
        sem_post(&mutex);

        free(entry);
    }
    else if (strcmp(command, "get") == 0){
        if (argc != 3){
            printf("Incorrect number of arguments entered.\nUsage: ./test get key\n");
            fclose(data); fclose(index);
            return -1;
        }
        // Don't need any semaphores here - writing processes write to a temp file then rename so no issues with reading
        get(data, index, argv[2], 0);

    }
    else if (strcmp(command, "ts") == 0){
        if (argc != 3){
            printf("Incorrect number of arguments entered.\nUsage: ./test ts key\n");
            fclose(data); fclose(index);
            return -1;
        }

        // Don't need any semaphores here - writing processes write to a temp file then rename so no issues with reading
        get(data, index, argv[2], 1);

    }
    else if (strcmp(command, "del") == 0){
        if (argc != 3){
            printf("Incorrect number of arguments entered.\nUsage: ./test del key\n");
            fclose(data); fclose(index);
            return -1;
        }
        char value[] = "null";
        Pair* entry = malloc(sizeof(Pair));
        // Use semaphores to ensure exclusive write
        sem_wait(&mutex);
        set(data, index, argv[2], value, entry, 1);
        sem_post(&mutex);
        free(entry);
    }
    else if (strcmp(command, "help") == 0){
        printf("Usage:\n");
        printf("./kvdb set key value\tSets a key value pair in the database\n");
        printf("./kvdb get key\t\tGets the value correspoding to the entered key from the database\n");
        printf("./kvdb ts key\t\tReturns the timestamp that this key was first and last set.\n");
        printf("./kvdb del key\t\tDeletes a key value pair from the database\n");
    }
    else{
        printf("Unknown command entered. For help type ./kvdb help\n");
    }
    sem_destroy(&mutex);
    fclose(index);
    fclose(data);
    return 0;
}

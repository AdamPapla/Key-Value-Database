
# Key Value Database Implemented in C  


## Outline

This program implements a simple key value database according to the specifications outlined in the technical test brief.  
It's dependencies are limited to basic, standard C libraries. It implements the database as follows:  
- Key value pairs are stored in order, with the first two letters used for ordering (alphabetically). The timestamps and sizes of the key are also stored. All storage is contiguous in the binary file to maximise storage efficiency.
- To improve performance, an index file is also used. This contains the first two letters of each key in the database, along with the start and end file offset of the KV entries beginning with those letters. This can be used to quickly get KV pairs and timestamps from the `data.bin` file without needing to traverse the entire file. Each entry in the `index.bin` file only uses 10 bytes of memory, so the improvement in performance comes at low overhead. 
- Semaphores are used to ensure exclusive write access for concurrency. Read operations (get, ts) need no protection as all write operations write to a temp file then rename temp file. So there is no danger of reading and writing concurrently. 
- Max key and value sizes are defined in **definitions.h**. These are present to prevent overflow, and can be modified by the user. 

## Limitations

Some limitations to this implementation are as follows:  
- Since the database is written in binary to improve performance, portability of a written database between different architectures may cause issues.
- To set a new value or delete an existing key, the database must be rewritten. There is no straightforward way to rewrite/insert a line in `.bin` files. This becomes expensive as the database grows. To remedy this, the index file can be expanded to point to different `data.bin` files depending on index. This means that the size of file to be rewritten can remain small. For even larger databases, a tree of index files may be implemented, and a more sophisticated index employed. 
- Database has vulnerabilities. Using double quotes or terminating characters in setting a key can result in undefined behaviour, and could be used maliciously.

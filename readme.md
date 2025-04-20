experimental version of memory manager allocator
based on AVL-tree structures(set, map)

----implemented functions----
-malloc
-calloc
-realloc
-free

-logging


avg. time complexity: O(logn) due to it was made on AVL-tree

usage example:
1)compile with corresponding command
g++ -o malloc.so -fPIC -shared malloc.cc

2)set LD_PRELOAD to execute the program with this allocator using command(execute ls example):
LD_PRELOAD=./malloc.so ls

3)enjoy! after successful executrion u will get in ur log file ur allocation records

/*
* references :
* simple-linear mem manager and its integration to linux
* https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory
* allocator concept
* https://habr.com/ru/articles/435698/
*/

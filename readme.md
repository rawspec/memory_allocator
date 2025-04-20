experimental version of memory manager allocator<br />
based on AVL-tree structures(set, map)<br />

----implemented functions----<br />
-malloc<br />
-calloc<br />
-realloc<br />
-free<br />

-logging<br />


avg. time complexity: O(logn) due to it was made on AVL-tree<br />

usage example:<br />
1) compile with corresponding command<br />
g++ -o malloc.so -fPIC -shared malloc.cc<br />

2) set LD_PRELOAD to execute the program with this allocator using command(execute ls example):<br />
LD_PRELOAD=./malloc.so ls<br />

3) enjoy! after successful executrion u will get in ur log file ur allocation records<br /> <br /> 

references :<br />
* simple-linear mem manager and its integration to linux<br />
https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory<br />

* allocator concept<br />
https://habr.com/ru/articles/435698/<br />

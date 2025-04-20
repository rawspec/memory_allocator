
#include <stdio.h> /* used : printf log */

#include <cstdint> /* used : uintptr_t */

#include "structs.hh"/* used : balanced tree memory managment 
almost every request for O(LogN),
storing map for block <ptr : size> */

#include <unistd.h> /* used : one-time allocation( memory also might be allocated one-time statically ) */

#include <pthread.h> /* used : pthread mutex locks */

/* logger includes(ChatGPT-made) */
#include <fcntl.h> /* used : open */
#include <stdarg.h> /* used : va */
#include <string.h> /* used : strlen */

/* constants */
#define KB 1024
#define MB KB * 1024
#define GB MB * 1024

#define ALLOCATION_SIZE 512 * MB
#define ALLOCATION_AMT 1024

#define LOG_FILE "/home/<usr>/Desktop/malloc/impl/mem_log.txt"

pthread_mutex_t global_malloc_lock;

struct mem_block_t
{
    size_t size_in_bytes;
    void* start_address;

    bool operator<( const mem_block_t& other ) const noexcept
    {
        if ( size_in_bytes < other.size_in_bytes )
            return true;

        /*
        * note: make such comparation
        * so that mem allocator will try to find appropriate memory block
        * at the begining of the contiguous space
        */
        if ( size_in_bytes == other.size_in_bytes )
            return start_address < other.start_address;

        return false;
    }

    /* btw I think c_set structure should implement other operators based on above one as std::set does it */
    bool operator==( const mem_block_t& other ) const noexcept
    {
        return size_in_bytes == other.size_in_bytes &&
            start_address == other.start_address;
    }

    bool operator!=( const mem_block_t& other ) const noexcept
    {
        return !( *this == other );
    }

    bool operator>( const mem_block_t& other ) const noexcept
    {
        return other < *this;
    }

    bool operator<=( const mem_block_t& other ) const noexcept
    {
        return !( other < *this );
    }

    bool operator>=( const mem_block_t& other ) const noexcept
    {
        return !( *this < other );
    }
};

class c_mem
{
private:
    size_t total_mem;
    size_t free_mem;

    c_set<mem_block_t, ALLOCATION_AMT> free_blocks;
    c_map<void*, size_t, ALLOCATION_AMT> allocated_map;
public:
    c_mem( size_t sz = 0 ) :
        free_blocks { mem_block_t { sz == 0 ? ALLOCATION_SIZE : sz, sbrk( sz == 0 ? ALLOCATION_SIZE : sz ) } },
        free_mem { sz == 0 ? ALLOCATION_SIZE : sz }, total_mem { sz == 0 ? ALLOCATION_SIZE : sz }
    {

    }

    void free( void* block_ptr )
    {
        auto it = allocated_map.find( block_ptr );
        if ( it )
        {
            auto block_sz = allocated_map[ block_ptr ];

            free_mem += block_sz;

            free_blocks.insert( { block_sz, block_ptr } );
            allocated_map.erase( block_ptr );


            return;
        }
        //else
        //   throw "free : bad dealloc";

    }

    size_t block_sz( void* p )
    {
        return allocated_map[ p ];
    }

    void* alloc( size_t sz_bytes )
    {

        auto free_block = free_blocks.lower_bound( { sz_bytes, nullptr } );

        if ( !free_block ) /* no block found - > allocation failed */
        {
            //throw "alloc : bad_alloc";
        }

        this->free_mem -= sz_bytes;

        mem_block_t ret { sz_bytes, free_block.get( ).start_address };

        allocated_map[ free_block.get( ).start_address ] = sz_bytes;

        /* extend the limitation of int(4 bytes) size so use uintptr_t which can represent pointer address */
        uintptr_t  start = reinterpret_cast< uintptr_t >( free_block.get( ).start_address );
        uintptr_t  end = start + sz_bytes;
        void* next_addr = reinterpret_cast< void* >( end );

        //void* next_addr = reinterpret_cast< char* >( free_block.get( ).start_address ) + sz_bytes;
        auto remaining_size = free_block.get( ).size_in_bytes - sz_bytes;

        auto ret_addr = free_block.get( ).start_address;

        free_blocks.erase( free_block.get( ) );

        if ( remaining_size > 0 )
        {
            free_blocks.insert( { remaining_size, next_addr } );
        }


        return ret_addr;
    }

    inline size_t capacity( ) const noexcept
    {
        return total_mem;
    }

    inline size_t free_sz( ) const noexcept
    {
        return free_mem;
    }
};


c_mem* mem = nullptr;

namespace logger
{
    static int log_fd = -1;

    void init_log( )
    {
        log_fd = open( "malloc_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644 );
        if ( log_fd == -1 )
        {
            const char* error_msg = "Failed to open log file!\n";
            write( STDERR_FILENO, error_msg, strlen( error_msg ) );
        }
    }

    void logf( const char* format, ... )
    {
        char buf[ 512 ];
        va_list args;
        va_start( args, format );
        vsnprintf( buf, sizeof( buf ), format, args );
        va_end( args );

        if ( log_fd == -1 ) init_log( );
        if ( log_fd != -1 )
        {
            write( log_fd, buf, strlen( buf ) );
        }
    }

    void log_alloc( void* ret, size_t sz_bytes )
    {
        float free_percent = 0;

        if ( mem->capacity( ) != 0 )
        {
            auto free_sz = mem->free_sz( );
            auto capacity = mem->capacity( );

            free_percent = ( float ) free_sz / ( float ) capacity;
        }

        logf( "[malloc] size=%zu, allocated at %p, total_mem=%zu, free_mem=%zu, free%=%.5f\n",
            sz_bytes,
            ret,
            mem->capacity( ),
            mem->free_sz( ),
            free_percent );
    }

    void log_freed( void* p, size_t mem_before, size_t mem_after )
    {
        logf( "[malloc] freed block at =%p, block_sz=%zu\n", p, ( mem_after - mem_before ) );
    }

    void log_c_allocated( void* p )
    {
        logf( "[malloc] c-allocated at%p\n", p );
    }

    void log_reallocated( void* p )
    {
        logf( "[malloc] reallocated at%p\n", p );
    }
}

extern "C" void* malloc( size_t sz_bytes )
{

    pthread_mutex_lock( &global_malloc_lock );

    if ( mem == nullptr )
    {
        static c_mem memory( ALLOCATION_SIZE );
        mem = &memory;
    }

    void* ret = mem->alloc( sz_bytes );

    logger::log_alloc( ret, sz_bytes );

    pthread_mutex_unlock( &global_malloc_lock );

    return ret;
}

extern "C" void free( void* p )
{
    pthread_mutex_lock( &global_malloc_lock );

    auto mem_before = mem->free_sz( );
    mem->free( p );
    auto mem_after = mem->free_sz( );

    logger::log_freed( p, mem_before, mem_after );

    pthread_mutex_unlock( &global_malloc_lock );
}

/* realloc, calloc weren't tested */

extern "C" void* realloc( void* ptr, size_t new_size )
{
    /* better way :
    * we can extend size then rebalance tree, or look for new memory block, then copy all the info */

    void* new_ptr = malloc( new_size );
    memcpy( new_ptr, ptr, mem->block_sz( ptr ) );

    free( ptr );

    return new_ptr;
}

extern "C" void* calloc( size_t num, size_t sz_bytes )
{
    size_t size = num * sz_bytes;
    auto block = malloc( size );

    memset( block, 0, size );

    logger::log_c_allocated( block );

    return block;
}

/*
* references :
* simple-linear mem manager and its integration to linux
* https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory
* allocator concept
* https://habr.com/ru/articles/435698/
*/

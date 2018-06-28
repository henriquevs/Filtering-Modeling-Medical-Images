#include "appsupport.h"
#include "libgomp.h"

typedef struct {
    unsigned int start_addr;
    unsigned int size;
    unsigned int allocated;
} heap_handler_t;

heap_handler_t heap_handlers[10] LOCAL_SHARED;
unsigned char next_free_handler LOCAL_SHARED= 0;

#define shmem_next SHMEM_NEXT
#define MEMCHECK_MALLOCS
#define STACK_IN_SHARED

inline void print_shmem_utilization() {
    _printdecp("Heap occupation (in bytes) is",
            ((unsigned int) shmem_next) - SHARED_BASE);
}

#define SHMEM_CLUSTER_LOCATION 0

void shmalloc_init(unsigned int address) {
    shmem_next = SHARED_BASE + address;
    SHMEM_LOCK = (unsigned int) LOCKS(SHMALLOC_LOCK_ID);
}

#ifdef HEAP_HANDLERS

int heap_init(unsigned int heap_size)
{

    SHMEM_LOCK_WAIT();
    int ret;

    if((shmem_next + heap_size > (SHARED_BASE + SHARED_SIZE - LOCAL_SHARED_SIZE) )||
            ( shmem_next + heap_size > (SHARED_BASE + SHARED_SIZE - (LOCAL_SHARED_SIZE + STACK_SIZE * prv_num_procs)) ) || (next_free_handler > 9)) {
        pr("[ERROR] Shared malloc is out of Memory!", 0x0, PR_CPU_ID | PR_STRING | PR_NEWL);
        abort();
    }

    ret = next_free_handler;
    next_free_handler++;
    heap_handlers[ret].start_addr = shmem_next;
    heap_handlers[ret].size = heap_size;
    shmem_next += heap_size;


    SHMEM_LOCK_SIGNAL();

    return ret;

}

void heap_reinit(int handler) {
    heap_handlers[handler].allocated=0;
}

inline void * shmalloc(int handler, int bytes) {

    unsigned int ret;

    if (heap_handlers[handler].allocated + bytes > heap_handlers[handler].size){
        pr("[ERROR] Shared malloc is out of Memory for handler: ", handler, PR_DEC | PR_CPU_ID | PR_STRING | PR_NEWL);
        abort();
    }
    ret = heap_handlers[handler].start_addr + heap_handlers[handler].allocated;
    heap_handlers[handler].allocated += bytes;

    return (void *)ret;

}

#else
// TODO implement the REAL shmalloc in the clustered environment

inline void *
shmalloc(int size) {
    volatile void *ret;
    SHMEM_LOCK_WAIT();

    ret = (void *) shmem_next;
    shmem_next += size;

    SHMEM_LOCK_SIGNAL();

    /* FIXME do it faster!! */
#ifdef MEMCHECK_MALLOCS
    if ((((unsigned int) ret) + size
            > (SHARED_BASE + SHARED_SIZE - LOCAL_SHARED_SIZE))
#if defined(STACK_IN_SHARED)
            || (((unsigned int) ret) + size
                    > (SHARED_BASE + SHARED_SIZE
                            - (LOCAL_SHARED_SIZE + STACK_SIZE * prv_num_procs)))
#endif
            ) {
        pr("[ERROR] Shared malloc is out of Memory!", 0x0,
                PR_CPU_ID | PR_STRING | PR_NEWL);
        abort();
    }
#endif

    return (void *) ret;
}

#endif /*#ifdef HEAP_HANDLERS*/

void shfree(void *address) {
}

void dcache_flush() {
}

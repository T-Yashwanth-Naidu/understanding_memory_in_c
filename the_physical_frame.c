// Start inlcuding things
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include<fcntl.h>
#include<inttypes.h>

int main(void){

    char *heap_data = malloc(11);
    
    // Start poking the heap
    heap_data[0]='Y';
   
    printf("Virtual Address: %p\n",(void*)heap_data);

    uintptr_t virtual_address =(uintptr_t)heap_data;

    // Get the page size
    // syscong asks the system for runtime config value
    // _SC_PAGESIZE - ask for size of a memory page. _SC_ means - System Config
    long page_size = sysconf(_SC_PAGESIZE);
    printf("page size: %ld\n",page_size);
    
    uintptr_t virtual_page = virtual_address/page_size;
    printf("Virtual page on which this data is stored:%p\n",(void*)virtual_page);

    uintptr_t offset_in_page = virtual_address % page_size;
    printf("The offset: %lu\n",offset_in_page);

    // Lets openup the page-map

    int fd = open("/proc/self/pagemap",O_RDONLY);

    if(fd<0)
    {
        perror("Run wid sudo bruh");
        return 1;
    }

    // Lets check the entry now
    // Yes, It was the first time I realize sizeof(entry) == sizeof entry and is valid!
    // pread - Position read - Read from a file at specific offset without sekeing there first.
    // fd - The file descriptor
    // &entry - Where to put the bytes
    // sizeof entry - how many bytes to read
    // virtual_page * sizeof entry - the offset in the file to read
    uint64_t entry;
    if(pread(fd,&entry, sizeof entry, virtual_page*sizeof entry) != sizeof entry  )
    {
        perror("pread");
        return 1;
    }
    close(fd);


    // Get the 63rd bit
    // Bit 63 (highest bit) is the "present" flag . 
    // 1 Means, page in mem, else not yet
    if(!(entry & (1ULL<<63) )){
    printf("Page aint in RAM yet, hold up\n");
    return 0;
    }
    
    // Extract the physical frame number(PFN) from packed entry,
    // The PFN lives in bits 0 -54. 
    // The logic tells which physical page frame in RAM backs your virtual page
    uint64_t page_frame_number = entry & ((1ULL<<55)-1);

    // Turn the PFN into a actual physical address
    uint64_t page_physical_address = page_frame_number * page_size + offset_in_page;

    printf("Physical Frame Number ( In RAM): 0x%" PRIx64 "\n", page_frame_number );
    printf("Physical Address (In RAM):  0x%" PRIx64 "\n", page_physical_address);

    // Aint leakin out memory
    free(heap_data);
    return 0;

}

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(void){
    
    // Assign 10 blocks of memory
    void *aPtr = malloc(10);
    
    // Malloc Return
    printf("malloc returned :%p\n",aPtr);

    // sbrk(0) return current break
    printf("current break: %p\n", sbrk(0));
    
    // pointer subtraction is defined between pointer of same type.
    // Since char is 1 byte, the result is count of bytes. 
    printf("gap (break-aPtr): %ld bytes\n", (long)((char*)sbrk(0)-(char*)aPtr));

    // Free dynamically allocated memory
    free(aPtr);
    return 0;


}


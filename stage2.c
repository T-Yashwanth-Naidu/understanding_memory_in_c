#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    char *break_start = (char *)sbrk(0);
    printf("start break: %p\n", break_start);

    for (int request = 0; request < 8; request++) {
        void *block = malloc(1000);
        char *break_current = (char *)sbrk(0);
        printf("after malloc #%d: block=%p  break=%p  grown=%ld bytes\n",
               request, block, (void *)break_current,
               (long)(break_current - break_start));
    }
    return 0;
}

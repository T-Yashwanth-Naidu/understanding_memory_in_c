#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main(){

   // Assign some memory using malloc
   char *store_data = malloc(11);
    
   if(!store_data)
   {
       return 1;
   }

   memcpy(store_data, "0123456789",11);
 
   // Malloc returns the address that is in the heap region for that data which is assigned which is created on heap. 
   printf("Data stored: %s at memory location: %p\n", store_data, (void*)store_data);

   free(store_data);
   return 0;
}

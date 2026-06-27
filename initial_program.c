#include<stdio.h>
#include<stdlib.h>

int main(){

   // Assign some memory using malloc
   char *store_data = malloc(11);
   char *source = "0123456789";
   if(!store_data)
   {
       return 1;
   }

   for(int i=0;i<11;i++)
   {
       store_data[i] = source[i];
   }

 
   // Malloc returns the address that is in the heap region for that data which is assigned which is created on heap. 
   printf("Data stored: %s at memory location: %p\n", store_data, (void*)store_data);

   free(store_data);
   return 0;
}

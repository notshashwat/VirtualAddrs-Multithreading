#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>


#define FETCH_BIT(X,Y) (X & ((uint64_t)1<<Y)) >> Y
//this macro inspects a particular bit of a number
const int __endian_bit = 1;
#define is_bigendian() ( (*(char*)&__endian_bit) == 0 )

int i, c, pid, status;
unsigned long virt_addr; 
uint64_t read_val, pagemap_loc;
char path_pm [0x100] = {};//path to proc/pid/pagemap
FILE * f;
char *end;

int read_pagemap(char * path_pm, unsigned long virt_addr);

int main(int argc, char ** argv){
   if(argc!=3){
      printf("Enter arguments- PID and virtual address\n");
      return -1;
   }
   if(!memcmp(argv[1],"self",sizeof("self"))){
   //check if the entered argument for pid is self
      sprintf(path_pm, "/proc/self/pagemap");
      
      pid = -1;
   }
   else{
         pid = strtol(argv[1],&end, 10);
         if (end == argv[1] || *end != '\0' || pid<=0){ 
            printf("PID must be a positive number or 'self'\n");
            return -1;
            }
       }
 
       if(pid!=-1){
         sprintf(path_pm, "/proc/%u/pagemap", pid);
       }
       

       virt_addr=strtol(argv[2],NULL,16);
//convert the virt addrr given to hex and send it to find the correspoing pagemap below
	read_pagemap(path_pm,virt_addr);
        
   return 0;
}

int read_pagemap(char * path_pm, unsigned long virt_addr){
   f = fopen(path_pm, "rb");
   if(!f){
      return -1;
   }
   //virtual address is divided by page size to get the page number in virt_addr and 
   //multiplied by size of one entry in page to essentially get the location in pagemap
   pagemap_loc = virt_addr / getpagesize() * 8;
   status = fseek(f, pagemap_loc, SEEK_SET);
   if(status){
      return -1;
   }
   read_val = 0;
   unsigned char c_buf[8];
   for(i=0; i < 8; i++){
      c = getc(f);
      if(c==EOF){
         printf("\nReached end of the file\n");
         return 0;
      }
      if(is_bigendian())
           c_buf[i] = c;
      else
           c_buf[8 - i - 1] = c;
   }
   for(i=0; i < 8; i++){
      read_val = (read_val << 8) + c_buf[i];
   }
   printf("PTE: %llx\n",(unsigned long long)read_val );
   if(FETCH_BIT(read_val,63))   //if the 63rd bit is 1 then pfn exixts
      printf("PFN: 0x%llx\n",(unsigned long long) (read_val & 0x7FFFFFFFFFFFFF)); //0-54 bits are PFN
   else//page is not present
      printf("Vaddr: 0x%lx, Page not present\n",virt_addr);
   if(FETCH_BIT(read_val,62))
   //if 62th bit is 1 then page is swapped
   	{printf("BIT 62: page is swapped\n");
//if the page is swapped bits 0-5 give the swap type
	printf("swap type: 0x%llx\n",(unsigned long long)(read_val & 0x1F));
//if the page is swapped bits 5-54 is swap offset
	printf("swap offset: 0x%llx\n",(unsigned long long)(read_val & 0x7FFFFFFFFFFFE0));
}
   if(FETCH_BIT(read_val,55))
   //if bit 55 is 1 then pte is soft dirty
   	printf("BIT 55 : pte is soft dirty\n");


   fclose(f);
   return 0;
}

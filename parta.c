#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>



const int __endian_bit = 1;
#define is_bigendian() ( (*(char*)&__endian_bit) == 0 )
//this macro is to check endianity later in the code when we are reading from pagemap
int i, c, pid, status;
unsigned long virt_addr; 
uint64_t read_val, pagemap_loc;
char path_pm [0x100] = {};//this is path to proc/pid/pagemap
char path_m [0x100] = {};//this is path to proc/pid/map
FILE * f;
char *end;

int read_pagemap(char * path_pm, unsigned long virt_addr);

int main(int argc, char ** argv){
   if(argc!=2){
      printf("Enter argument- PID\n");
      return -1;
   }
   if(!memcmp(argv[1],"self",sizeof("self"))){
   //check if the entered argument for pid is self
      sprintf(path_pm, "/proc/self/pagemap");
      sprintf(path_m, "/proc/self/maps");
      //contruct the paths
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
   	 sprintf(path_m, "/proc/%u/maps", pid);
       }
       
       //iterating through all the lines in /proc/pid/maps
	    FILE *in_file = fopen(path_m, "r");

	    struct stat sb;
	    stat(path_m, &sb);

	    char *file_contents = malloc(sb.st_size);
//iterating through all the lines until EOF is reached
	    while (fscanf(in_file, "%[^\n] ", file_contents) != EOF) {
	    	char *temp = file_contents;
		while (*temp!='-') temp++;//go up to the dash(-) and put \0 in place of dash to essentaially read only the starting address before dash in each line
		*temp='\0';
		
		//now file_contents contains the virtual address text
		virt_addr = strtol(file_contents, NULL, 16);
		//convert it to hex 
	    	read_pagemap(path_pm,virt_addr);
	    	//send it for finding the corresponding pfn
		}
		
		fclose(in_file);
        
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
   if((read_val & ((uint64_t)1<<63)) >> 63)
      //if the 64rd bit is 1 then pfn exixts
      printf("Vaddr: 0x%lx, PFN: 0x%llx\n" ,virt_addr,(unsigned long long) (read_val & 0x7FFFFFFFFFFFFF)); //0-55 bits are PFN
   else
   //else page is not present
      printf("Vaddr: 0x%lx, Page not present\n",virt_addr);
   fclose(f);
   return 0;
}

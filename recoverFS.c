//
//  recoverFS.c
//  file_system
//
//  Created by Darrell Shi on 11/13/16.
//  Copyright © 2016 darrellshi. All rights reserved.
//

#include "simpleFS.h"

char get_bit_from_bitmap(char * bitmap, int index) {
  int byte_index = index / 8;
  int offset = index % 8;

  char byte = 0;
  switch (offset) {
    case 0:
    byte = bitmap[byte_index] & bitmap_mask0;
    break;
    case 1:
    byte = bitmap[byte_index] & bitmap_mask1;
    break;
    case 2:
    byte = bitmap[byte_index] & bitmap_mask2;
    break;
    case 3:
    byte = bitmap[byte_index] & bitmap_mask3;
    break;
    case 4:
    byte = bitmap[byte_index] & bitmap_mask4;
    break;
    case 5:
    byte = bitmap[byte_index] & bitmap_mask5;
    break;
    case 6:
    byte = bitmap[byte_index] & bitmap_mask6;
    break;
    case 7:
    byte = bitmap[byte_index] & bitmap_mask7;
    break;
  }
  if (byte) {
    return 1;
  } else {
    return 0;
  }
}

int main(int argc, char **argv)
{
 char path[100];

 printf("enter file system image path: ");
 scanf("%s", path);
 getchar();

 fp = fopen(path, "r");
 if (fp == NULL) {
   printf("[invalid path for image. image cannot be opened]\n");
   return 1;
 }

 fseek(fp, 0L, SEEK_END);
 long size = ftell(fp);

 unsigned char * mem = (unsigned char *) malloc(size*sizeof(char));

   fseek(fp, SEEK_SET, 0); // seek to beginning of the file
   
   long bytes_read = fread(mem, 1, size, fp);
   
   if (bytes_read == 0) {
     printf("[failed to read data]\n");
     fclose(fp);
     return 0;
   }
   
   char * inode_bm = (char *)mem + 2*BLOCK_SIZE;
   struct inode * inode_table = (struct inode *)(mem + 3*BLOCK_SIZE);
   unsigned char * data_blocks = (unsigned char *)(mem + 8*BLOCK_SIZE);

   FILE * out_file;
   int i;
   int count = 0;
   
   for (i = 1; i < 8*BLOCK_SIZE-1; i++) {
    char bit = get_bit_from_bitmap(inode_bm, i);

     if (bit != 0) {
       struct inode inode = inode_table[i+1];
       if (inode.i_mode == S_REG) {
               // is a file and needs to be written
         char filename[17];
         sprintf(filename, "recover_file%d", i+1);
         out_file = fopen(filename, "w");

         char * data_addr = (char *) &data_blocks[inode.i_block[0]*BLOCK_SIZE];
         fwrite(data_addr, 1, inode.i_size, out_file);
         fclose(out_file);
         count++;
       }
     }
   }

   printf("[%d files have been recovered]\n", count);
   
   return 0;
 }

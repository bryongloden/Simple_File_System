Task 2: Questions 
1. How many files are possible with this file system?
Total number of inodes: 5 * BLOCK_SIZE / sizeof(struct inode) = 40
40 - 1 (root directory) = 39
This file system can have up to 39 files.

2. What is the largest possible file size in bytes?
8 * BLOCK_SIZE = 4096 bytes

3. How many bytes of data can the largest "drive" be made with this file system?
The largest size of the drive depends on the size of datablock bitmap
Since the datablock bitmap takes 1 block (512 bytes), that is 4096 bits, the system can have up to 4096 datablocks. Therefore the largest size would be 4096 * BLOCK_SIZE = 2,097,152 bytes = 2 megabytes.

4. How many directory entries can you have in a single directory?
If only use the one block of data (like in this lab):
BLOCK_SIZE / sizeof(struct directory_entry) = 8
If use all 8 blocks of data:
8 * BLOCK_SIZE / sizeof(struct directory_entry) = 64

5. Can we have multiple disk accesses concurrently with this file system? Explain why or why not?
No. This file system reads from an image file. Reading and writing concurrently to the image file could cause inconsistency and create weird bugs.

Task 3: Bonus Forensics task
Since only the superblock is wiped cleaned, we still have access to inode bitmap, inode table and data blocks.
Read through the inode bitmap. For each bit 1, read the corresponding inode from inode table. If the inode points to a directory, ignore (since we are not recovering the original directory structure, only the files). If the inode points to a file, create a new file in the real life system and write out the contents from data blocks.
/*
 * extmem.h
 * Zhaonian Zou
 * Harbin Institute of Technology
 * Jun 22, 2011
 */

#ifndef EXTMEM_H
#define EXTMEM_H

#define BLOCK_AVAILABLE 0
#define BLOCK_UNAVAILABLE 1

// Info prefix for printing
#define READ_PREFIX "Read"
#define WRITE_PREFIX "Write"

typedef struct tagBuffer {
    unsigned long numIO; /* Number of IO's*/
    size_t bufSize; /* Buffer size*/
    size_t blkSize; /* Block size */
    size_t numAllBlk; /* Number of blocks that can be kept in the buffer */
    size_t numFreeBlk; /* Number of available blocks in the buffer */
    unsigned char *data; /* Starting address of the buffer */
} Buffer;

/* Initialize a buffer with the specified buffer size and block size.
 * If the initialization fails, the return value is NULL;
 * otherwise the pointer to the buffer.
 */
Buffer *initBuffer(size_t bufSize, size_t blkSize, Buffer *buf);

// embeded initBuffer
Buffer *InitBuffer(size_t bufSize, size_t blkSize, Buffer *buf);

/* Free the memory used by a buffer. */
void freeBuffer(Buffer *buf);

/* Apply for a new block from a buffer.
 * If no free blocks are available in the buffer,
 * then the return value is NULL; otherwise the pointer to the block.
 */
unsigned char *getNewBlockInBuffer(Buffer *buf);

// embeded getNewBlockInBuffer
unsigned char *GetNewBlockInBuffer(Buffer *buf);

/* Set a block in a buffer to be available. */
void freeBlockInBuffer(unsigned char *blk, Buffer *buf);

/* Drop a block on the disk */
int dropBlockOnDisk(unsigned int addr);

/* Read a block from the hard disk to the buffer by the address of the block. */
unsigned char *readBlockFromDisk(unsigned int addr, Buffer *buf);

// embeded readBlockFromDisk
unsigned char *ReadBlockFromDisk(unsigned int addr, Buffer *buf, int *IOtimes);

/* Read a block in the buffer to the hard disk by the address of the block. */
int writeBlockToDisk(unsigned char *blkPtr, unsigned int addr, Buffer *buf);

// embeded writeBlockToDisk
void WriteBlockToDisk(unsigned char *blkPtr, unsigned int addr, Buffer *buf, int *IOtimes);

#endif // EXTMEM_H

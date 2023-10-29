#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extmem.h"

// Data about relation R
#define RBEGIN      1
#define REND        16
#define RSIZE       16
#define RBUFFER     49

// Data about relation S
#define SBEGIN      17
#define SEND        48
#define SSIZE       32
#define SBUFFER     65

// Data about buffer block
#define ATTRSIZE    4
#define LINENUM     7
#define LINESIZE    8
#define BLOCKSIZE   64
#define BLOCKNUM    8

// Where to write the result
#define OFFSET1     100
#define OFFSETR2     301
#define OFFSETS2     317
#define OFFSET3     401
#define OFFSET4     402

int IOTimes = 0;

// utils
#define NEXT(p) ((char *)(p) + LINENUM * LINESIZE)
#define SETNEXT(p, i) (sprintf(NEXT(p), "%d", (i)))
#define LINE(p, i) ((char *)(p) + (i) * LINESIZE)
#define ATTR(p, i) (atoi(((char *)(p) + (i) * ATTRSIZE)))
#define GROUP(p, off) ((p) + (off) * groupSize)
#define BLOCK(p, off) ((p) + (off))
#define BLOCKEND(p) (*(p) == '\0')
// extmem
#define GETBLOCK(p) ((p) = GetNewBlockInBuffer(&buf))
#define READ(addr) (ReadBlockFromDisk((addr), &buf, &IOTimes))
#define WRITE(blk, addr) (WriteBlockToDisk((blk), (addr), &buf, &IOTimes))
#define FREE(blk) (freeBlockInBuffer((blk), &buf))
// function tools
int cmp(const void *a, const void *b);
void mergeSort(int start, int nBlocks, int tmpStart);

// task 1
// Relation Selection Algorithm based on linear search
void task1();

// task 2
// TPMMS(Two-Pass Multiway Merge Sort)
void task2();

int main(int argc, char *argv[])
{
    task1();
    task2();

    return 0;
}

int cmp(const void *a, const void *b)
{
    int a1 = ATTR(a, 0), a2 = ATTR(a, 1);
    int b1 = ATTR(b, 0), b2 = ATTR(b, 1);
    return (a1 == b1) ? (a2 - b2) : (a1 - b1);
}

void mergeSort(int start, int nBlocks, int tmpStart)
{
    Buffer buf;
    int groupSize = 1, originStart = start;

    InitBuffer(520, 64, &buf);

    while (groupSize < nBlocks)
    {
        int writeTimes = 0;
        for (int i = start; i < start + nBlocks; i += (BLOCKNUM - 1) * groupSize)
        {
            unsigned char *wBlk, *rBlk[BLOCKNUM - 1] = {};
            int groupNum = 0, remain = 0, idx[BLOCKNUM - 1] = {}, hasRead[BLOCKNUM - 1] = {}, writeNum = 0;

            GETBLOCK(wBlk);
            for (int j = 0; j < BLOCKNUM - 1; j++)
            {
                if (GROUP(i, j) >= BLOCK(start, nBlocks))
                    break;
                rBlk[j] = READ(GROUP(i, j));
                groupNum++;
                remain++;
            }

            while (remain)
            {
                int min = -1;
                for (int j = 0; j < groupNum; j++)
                {
                    if (!rBlk[j])
                        continue;
                    if (min == -1)
                        min = j;
                    else if (cmp(LINE(rBlk[j], idx[j]), LINE(rBlk[min], idx[min])) < 0)
                        min = j;
                }
                memcpy(LINE(wBlk, writeNum++), LINE(rBlk[min], idx[min]++), LINESIZE);
                if (writeNum == LINENUM)
                {
                    SETNEXT(wBlk, BLOCK(tmpStart, writeTimes + 1));
                    WRITE(wBlk, BLOCK(tmpStart, writeTimes++));
                    GETBLOCK(wBlk);
                    writeNum = 0;
                }
                if (idx[min] == LINENUM || BLOCKEND(LINE(rBlk[min], idx[min])))
                {
                    FREE(rBlk[min]);
                    hasRead[min]++;
                    if (hasRead[min] == groupSize || BLOCK(GROUP(i, min), hasRead[min]) >= BLOCK(start, nBlocks))
                    {
                        rBlk[min] = NULL;
                        remain--;
                    }
                    else
                    {
                        rBlk[min] = READ(BLOCK(GROUP(i, min), hasRead[min]));
                        idx[min] = 0;
                    }
                }
            }
            if (writeNum)
                WRITE(wBlk, BLOCK(tmpStart, writeTimes++));
            else
                FREE(wBlk);
        }
        int tmp = start;
        start = tmpStart;
        tmpStart = tmp;
        groupSize *= (BLOCKNUM - 1);
    }

    if (start != originStart)
    {
        for (int i = start; i < start + nBlocks; i++)
        {
            unsigned char *blk = READ(i);
            WRITE(blk, BLOCK(originStart, i - start));
        }
    }

    freeBuffer(&buf);
}

void task1()
{
    Buffer buf;
    unsigned char *rblk, *wblk;

    IOTimes = 0;

    InitBuffer(520, 64, &buf);

    int writeNum = 0;
    int writeTimes = 0;

    for (int i = SBEGIN; i <= SEND; i++)
    {
        rblk = READ(i);
        for (int j = 0; j < LINENUM; j++)
        {
            if (BLOCKEND(LINE(rblk, j)))
                break;

            int C = ATTR(LINE(rblk, j), 0);
            int D = ATTR(LINE(rblk, j), 1);

            if (C == 107)
            {
                printf("(C=%d, D=%d)\n", C, D);
                if (!wblk)
                    GETBLOCK(wblk);
                memcpy(LINE(wblk, writeNum++), LINE(rblk, j), LINESIZE);
            }

            if (writeNum == 7)
            {
                sprintf(NEXT(wblk), "%d", BLOCK(OFFSET1, writeTimes + 1));
                WRITE(wblk, BLOCK(OFFSET1, writeTimes++));
                wblk = NULL;
                writeNum = 0;
            }
        }
        FREE(rblk);
    }

    if (wblk)
        WRITE(wblk, BLOCK(OFFSET1, writeTimes++));

    printf("IO times: %d\n\n\n", IOTimes);
    freeBuffer(&buf);
}

void task2()
{
    Buffer buf;
    int writeTimes = 0;

    IOTimes = 0;

    InitBuffer(520, 64, &buf);

    // first pass, sort each block
    for (int i = RBEGIN; i <= SEND; i++)
    {
        unsigned char *blk;

        blk = READ(i);

        int nTuples = 0;
        for (int j = 0; j < LINENUM; j++)
        {
            if (BLOCKEND(LINE(blk, j)))
                break;
            nTuples++;
        }
        qsort(blk, nTuples, LINESIZE, cmp);

        SETNEXT(blk, BLOCK(OFFSETR2, writeTimes + 1));
        WRITE(blk, BLOCK(OFFSETR2, writeTimes++));
    }

    freeBuffer(&buf);

    // second pass, merge sort
    mergeSort(OFFSETR2, RSIZE, RBUFFER);
    mergeSort(OFFSETS2, SSIZE, SBUFFER);

    printf("IO times: %d\n\n\n", IOTimes);
}

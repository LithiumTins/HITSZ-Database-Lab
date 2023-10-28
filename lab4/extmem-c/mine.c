#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extmem.h"

// Data about relation R
#define RBEGIN      1
#define REND        16
#define RSIZE       16

// Data about relation S
#define SBEGIN      17
#define SEND        48
#define SSIZE       32

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
#define LINE(p, i) ((char *)(p) + (i) * LINESIZE)
#define ATTR(p, i) ((char *)(p) + (i) * ATTRSIZE)
int cmp(const void *a, const void *b);
void mergeSort(int start, int nBlocks, int tmpStart);

// task 1
// Relation Selection Algorithm through linear search
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
    int a1 = atoi(ATTR(a, 0)), a2 = atoi(ATTR(a, 1));
    int b1 = atoi(ATTR(b, 0)), b2 = atoi(ATTR(b, 1));
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
            int groupNum = 0, remain = 0, idx[BLOCKNUM - 1] = {}, cnt[BLOCKNUM - 1] = {}, writeNum = 0;

            wBlk = GetNewBlockInBuffer(&buf);
            for (int j = 0; j < BLOCKNUM - 1; j++)
            {
                if (i + j * groupSize >= start + nBlocks)
                    break;
                rBlk[j] = ReadBlockFromDisk(i + j * groupSize, &buf, &IOTimes);
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
                    sprintf(NEXT(wBlk), "%d", tmpStart + writeTimes + 1);
                    WriteBlockToDisk(wBlk, tmpStart + writeTimes++, &buf, &IOTimes);
                    wBlk = GetNewBlockInBuffer(&buf);
                    writeNum = 0;
                }
                if (idx[min] == LINENUM)
                {
                    freeBlockInBuffer(rBlk[min], &buf);
                    cnt[min]++;
                    if (cnt[min] == groupSize || i + min * groupSize + cnt[min] >= start + nBlocks)
                    {
                        rBlk[min] = NULL;
                        remain--;
                    }
                    else
                    {
                        rBlk[min] = ReadBlockFromDisk(i + min * groupSize + cnt[min], &buf, &IOTimes);
                        idx[min] = 0;
                    }
                }
            }
            if (writeNum)
                WriteBlockToDisk(wBlk, tmpStart + writeTimes++, &buf, &IOTimes);
            else
                freeBlockInBuffer(wBlk, &buf);
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
            unsigned char *blk = ReadBlockFromDisk(i, &buf, &IOTimes);
            WriteBlockToDisk(blk, originStart + i - start, &buf, &IOTimes);
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
        rblk = ReadBlockFromDisk(i, &buf, &IOTimes);
        for (int j = 0; j < LINENUM; j++)
        {
            if (*(rblk + j * LINESIZE) == '\0')
                break;

            int C = atoi(ATTR(LINE(rblk, j), 0));
            int D = atoi(ATTR(LINE(rblk, j), 1));

            if (C == 107)
            {
                printf("(C=%d, D=%d)\n", C, D);
                if (!wblk)
                    wblk = GetNewBlockInBuffer(&buf);
                memcpy(LINE(wblk, writeNum++), LINE(rblk, j), LINESIZE);
            }

            if (writeNum == 7)
            {
                sprintf(NEXT(wblk), "%d", OFFSET1 + writeTimes + 1);
                WriteBlockToDisk(wblk, OFFSET1 + writeTimes++, &buf, &IOTimes);
                wblk = NULL;
                writeNum = 0;
            }
        }
        freeBlockInBuffer(rblk, &buf);
    }

    if (wblk)
        WriteBlockToDisk(wblk, OFFSET1 + writeTimes++, &buf, &IOTimes);

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

        blk = ReadBlockFromDisk(i, &buf, &IOTimes);

        int nTuples = 0;
        for (int j = 0; j < LINENUM; j++)
        {
            if (*(blk + j * LINESIZE) == '\0')
                break;
            nTuples++;
        }
        qsort(blk, nTuples, LINESIZE, cmp);

        sprintf(NEXT(blk), "%d", OFFSETR2 + writeTimes + 1);
        WriteBlockToDisk(blk, OFFSETR2 + writeTimes++, &buf, &IOTimes);
    }

    freeBuffer(&buf);

    // second pass, merge sort
    mergeSort(OFFSETR2, RSIZE, SEND + 1);
    mergeSort(OFFSETS2, SSIZE, SEND + 1 + RSIZE);

    printf("IO times: %d\n\n\n", IOTimes);
}

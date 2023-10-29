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
#define SORTR     301
#define SORTS     317
#define INDEXR      350
#define INDEXS      355
#define OFFSET3     120
#define OFFSET4     401

int IOTimes = 0;
int rIndexSize = 0, sIndexSize = 0;

// utils
#define NEXT(p) ((char *)(p) + LINENUM * LINESIZE)
#define SETNEXT(p, i) (sprintf(NEXT(p), "%d", (i)))
#define LINE(p, i) ((char *)(p) + (i) * LINESIZE)
#define ATTR(p, i) (atoi(((char *)(p) + (i) * ATTRSIZE)))
#define GROUP(p, off) ((p) + (off) * groupSize)
#define BLOCK(p, off) ((p) + (off))
#define BLOCKEND(p) (*(p) == '\0')
#define COPYLINE(p, q) (memcpy((p), (q), LINESIZE))
// extmem
#define GETBLOCK(p) ((p) = GetNewBlockInBuffer(&buf))
#define READ(addr) (ReadBlockFromDisk((addr), &buf, &IOTimes))
#define WRITE(blk, addr) (WriteBlockToDisk((blk), (addr), &buf, &IOTimes))
#define FREE(blk) (freeBlockInBuffer((blk), &buf))
// function tools
int cmp(const void *a, const void *b);
void mergeSort(int start, int nBlocks, int tmpStart);
void buildIndex(int start, int nBlocks, int indexStart, int *indexSize);

// task 1
// Relation Selection Algorithm based on linear search
void task1();

// task 2
// TPMMS(Two-Pass Multiway Merge Sort)
void task2();

// task 3
// Relation Selection Algorithm based on index
void task3();

// task 4
// Join Algorithm based on sorting
void task4();

int main(int argc, char *argv[])
{
    task1();
    task2();
    task3();
    task4();

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
                COPYLINE(LINE(wBlk, writeNum++), LINE(rBlk[min], idx[min]++));
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
            {
                SETNEXT(wBlk, BLOCK(tmpStart, writeTimes + 1));
                WRITE(wBlk, BLOCK(tmpStart, writeTimes++));
            }
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

void buildIndex(int start, int nBlocks, int indexStart, int *indexSize)
{
    Buffer buf;
    unsigned char *wBlk, *rBlk;

    InitBuffer(520, 64, &buf);

    int writeNum = 0;
    int writeTimes = 0;
    GETBLOCK(wBlk);
    for (int i = start; i < BLOCK(start, nBlocks); i++)
    {
        rBlk = READ(i);
        COPYLINE(LINE(wBlk, writeNum++), LINE(rBlk, 0));
        if (writeNum == LINENUM)
        {
            SETNEXT(wBlk, BLOCK(indexStart, writeTimes + 1));
            WRITE(wBlk, BLOCK(indexStart, writeTimes++));
            GETBLOCK(wBlk);
            writeNum = 0;
            (*indexSize)++;
        }
        FREE(rBlk);
    }
    if (writeNum)
    {
        SETNEXT(wBlk, BLOCK(indexStart, writeTimes + 1));
        WRITE(wBlk, BLOCK(indexStart, writeTimes++));
        (*indexSize)++;
    }

    freeBuffer(&buf);
}

void task1()
{
    Buffer buf;
    unsigned char *rBlk, *wBlk;

    IOTimes = 0;

    InitBuffer(520, 64, &buf);

    int writeNum = 0;
    int writeTimes = 0;

    for (int i = SBEGIN; i <= SEND; i++)
    {
        rBlk = READ(i);
        for (int j = 0; j < LINENUM; j++)
        {
            if (BLOCKEND(LINE(rBlk, j)))
                break;

            int C = ATTR(LINE(rBlk, j), 0);
            int D = ATTR(LINE(rBlk, j), 1);

            if (C == 107)
            {
                printf("(C=%d, D=%d)\n", C, D);
                if (!wBlk)
                    GETBLOCK(wBlk);
                COPYLINE(LINE(wBlk, writeNum++), LINE(rBlk, j));
            }

            if (writeNum == 7)
            {
                SETNEXT(wBlk, BLOCK(OFFSET1, writeTimes + 1));
                WRITE(wBlk, BLOCK(OFFSET1, writeTimes++));
                wBlk = NULL;
                writeNum = 0;
            }
        }
        FREE(rBlk);
    }

    if (wBlk)
    {
        SETNEXT(wBlk, BLOCK(OFFSET1, writeTimes + 1));
        WRITE(wBlk, BLOCK(OFFSET1, writeTimes++));
    }

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

        SETNEXT(blk, BLOCK(SORTR, writeTimes + 1));
        WRITE(blk, BLOCK(SORTR, writeTimes++));
    }

    freeBuffer(&buf);

    // second pass, merge sort
    mergeSort(SORTR, RSIZE, RBUFFER);
    mergeSort(SORTS, SSIZE, SBUFFER);

    printf("IO times: %d\n\n\n", IOTimes);
}

void task3()
{
    buildIndex(SORTR, RSIZE, INDEXR, &rIndexSize);
    buildIndex(SORTS, SSIZE, INDEXS, &sIndexSize);

    Buffer buf;
    unsigned char *rBlk, *wBlk = NULL;

    InitBuffer(520, 64, &buf);

    int searchStart = 0;
    for (int i = INDEXS; i < BLOCK(INDEXS, sIndexSize); i++)
    {
        rBlk = READ(i);
        int C;
        for (int j = 0; j < LINENUM && !BLOCKEND(LINE(rBlk, j)); j++)
        {
            C = ATTR(LINE(rBlk, j), 0);
            if (C > 107)
            {
                break;
            }
            else if (C == 107)
            {
                searchStart = (searchStart) ? searchStart : (SORTS + (i - INDEXS) * LINENUM + j);
                break;
            }
            else
            {
                searchStart = SORTS + (i - INDEXS) * LINENUM + j;
            }
        }
        FREE(rBlk);
        if (C >= 107)
            break;
    }

    IOTimes = 0;

    int writeNum = 0;
    int writeTimes = 0;
    for (int i = searchStart; i < BLOCK(SORTS, SSIZE); i++)
    {
        rBlk = READ(i);
        for (int j = 0; j < LINENUM; j++)
        {
            if (BLOCKEND(LINE(rBlk, j)))
                break;

            int C = ATTR(LINE(rBlk, j), 0);
            int D = ATTR(LINE(rBlk, j), 1);

            if (C == 107)
            {
                printf("(C=%d, D=%d)\n", C, D);
                if (!wBlk)
                    GETBLOCK(wBlk);
                COPYLINE(LINE(wBlk, writeNum++), LINE(rBlk, j));
            }
            else if (C > 107)
            {
                if (wBlk)
                {
                    SETNEXT(wBlk, BLOCK(OFFSET3, writeTimes + 1));
                    WRITE(wBlk, BLOCK(OFFSET3, writeTimes++));
                }
                printf("IO times: %d\n\n\n", IOTimes);
                freeBuffer(&buf);
                return;
            }

            if (writeNum == 7)
            {
                SETNEXT(wBlk, BLOCK(OFFSET3, writeTimes + 1));
                WRITE(wBlk, BLOCK(OFFSET3, writeTimes++));
                wBlk = NULL;
                writeNum = 0;
            }
        }
        FREE(rBlk);
    }

    if (wBlk)
    {
        SETNEXT(wBlk, BLOCK(OFFSET3, writeTimes + 1));
        WRITE(wBlk, BLOCK(OFFSET3, writeTimes++));
    }

    printf("IO times: %d\n\n\n", IOTimes);
    freeBuffer(&buf);
}

void task4()
{
    // R and S have been sorted in task2
    Buffer buf;
    unsigned char *rBlk, *sBlk, *wBlk = NULL;

    InitBuffer(520, 64, &buf);

    int writeNum = 0;
    int writeTimes = 0;
    int start = SORTS;
    int curS = start;
    sBlk = READ(start);
    GETBLOCK(wBlk);
    for (int i = SORTR; i < BLOCK(SORTR, RSIZE); i++)
    {
        rBlk = READ(i);
        for (int y = 0; y < LINENUM && !BLOCKEND(LINE(rBlk, y)); y++)
        {
            int nextR = 0, changeStart = 0;
            for (int j = start; j < BLOCK(SORTS, SSIZE); j++)
            {
                if (j != curS)
                {
                    FREE(sBlk);
                    sBlk = READ(j);
                }
                for (int k = 0; k < LINENUM && !BLOCKEND(LINE(sBlk, k)); k++)
                {
                    int A = ATTR(LINE(rBlk, y), 0), C = ATTR(LINE(sBlk, k), 0);
                    if (A == C)
                    {
                        if (!changeStart)
                        {
                            start = j;
                            changeStart = 1;
                        }
                        COPYLINE(LINE(wBlk, writeNum++), LINE(sBlk, k));
                        COPYLINE(LINE(wBlk, writeNum++), LINE(rBlk, y));
                        if (writeNum == (LINENUM / 2) * 2)
                        {
                            SETNEXT(wBlk, BLOCK(OFFSET4, writeTimes + 1));
                            WRITE(wBlk, BLOCK(OFFSET4, writeTimes++));
                            GETBLOCK(wBlk);
                            writeNum = 0;
                        }
                    }
                    else if (A < C)
                    {
                        nextR = 1;
                        break;
                    }
                }
                if (nextR)
                    break;
            }
        }
        FREE(rBlk);
    }
    if (writeNum)
    {
        SETNEXT(wBlk, BLOCK(OFFSET4, writeTimes + 1));
        WRITE(wBlk, BLOCK(OFFSET4, writeTimes));
    }

    printf("Join times: %d\n", writeTimes * 3 + writeNum / 2);
    printf("IO times: %d\n\n\n", IOTimes);
    freeBuffer(&buf);
}
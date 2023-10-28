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
#define LINENUM     7
#define LINESIZE    8
#define BLOCKSIZE   64

// Where to write the result
#define OFFSET1     100
#define OFFSET2     301
#define OFFSET3     401
#define OFFSET4     402

// task 1
// Relation Selection Algorithm through linear search
void task1();

int main(int argc, char *argv[])
{
    task1();

    return 0;
}

void task1()
{
    Buffer buf;
    unsigned char *rblk, *wblk;
    int IOtimes = 0;

    InitBuffer(520, 64, &buf);

    int writenum = 0;
    int writetimes = 0;

    for (int i = SBEGIN; i <= SEND; i++)
    {
        rblk = ReadBlockFromDisk(i, &buf, &IOtimes);
        for (int j = 0; j < LINENUM; j++)
        {
            if (*(rblk + j * LINESIZE) == '\0')
                break;

            int C = atoi((char *)rblk + j * LINESIZE + 0);
            int D = atoi((char *)rblk + j * LINESIZE + 4);

            if (C == 107)
            {
                printf("(C=%d, D=%d)\n", C, D);
                if (!wblk)
                    wblk = GetNewBlockInBuffer(&buf);
                memcpy(wblk + writenum++ * LINESIZE, rblk + j * LINESIZE, LINESIZE);
            }

            if (writenum == 7)
            {
                sprintf((char *)wblk + writenum * LINESIZE, "%d", OFFSET1 + writetimes + 1);
                WriteBlockToDisk(wblk, OFFSET1 + writetimes++, &buf, &IOtimes);
                wblk = NULL;
                writenum = 0;
            }
        }
        freeBlockInBuffer(rblk, &buf);
    }

    if (wblk)
        WriteBlockToDisk(wblk, OFFSET1 + writetimes++, &buf, &IOtimes);

    printf("IO times: %d\n", IOtimes);
    freeBuffer(&buf);
}
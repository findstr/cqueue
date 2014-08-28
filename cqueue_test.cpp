// cqueue_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <assert.h>
#include "cqueue.h"

struct cqueue *queue;

#define	 CS_VALUE	0x4fa82228

DWORD WINAPI pusher(LPVOID lp)
{
        int value;
        char path[MAX_PATH];
        int cnt = 0x100;
        sprintf(path, "C:\\Test\\%d%d.bin", 1, (unsigned long)lp);
        FILE *fp = fopen(path, "rb+");

        while (1) {
                if (fread(&value, sizeof(value), 1, fp) != 1)
                        break;
                cqueue_push(queue, &value);
        }

        return 0;
}

DWORD WINAPI poper(LPVOID lp)
{
        int i;
        char path[MAX_PATH];
        int value;
        int cnt = 5 * 1024 * 1024 / 4;

        sprintf(path, "C:\\Test\\%d%d.bin", 2, (unsigned long)lp);

        FILE *fp = fopen(path, "wb+");


        i = 0;
        while (cnt--) {
                cqueue_pop(queue, &value);
                fwrite(&value, sizeof(value), 1, fp);
        }

	fclose(fp);
        printf("pop -->%d\n", i);

        return 0;
}

unsigned long cs_check()
{
	FILE *fp1, *fp2;
	unsigned char value;
	unsigned long cs;

	cs = 0;

	fp1 = fopen("C:\\Test\\21.bin", "rb");
	fp2 = fopen("C:\\Test\\22.bin", "rb");

	while (1) {
		if (fread(&value, sizeof(value), 1, fp1) != 1)
			break;

		cs += value;
	}

	while (1) {
		if (fread(&value, sizeof(value), 1, fp2) != 1)
			break;

		cs += value;
	}

	if ((cs & 0xffffffff) == CS_VALUE)
		printf("congratulations, your are right, :) \n");
	else
		printf("sorry, your are wrong!\n");

	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
        HANDLE h[64];

        queue = cqueue_create(10, sizeof(int));

        int cnt;

        cnt = 3;

        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);


        memset(h, 0, sizeof(h));
        h[0] = CreateThread(NULL, 0, pusher, (LPVOID)1, 0, NULL);
        h[1] = CreateThread(NULL, 0, pusher, (LPVOID)2, 0, NULL);
        h[2] = CreateThread(NULL, 0, poper, (LPVOID)1, 0, NULL);
        h[3] = CreateThread(NULL, 0, poper, (LPVOID)2, 0, NULL);

        WaitForMultipleObjects(4, h, TRUE, -1);

	cs_check();

        printf("Test Finish\n");
        system("PAUSE");


	return 0;
}


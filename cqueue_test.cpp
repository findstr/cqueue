// cqueue_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include "assist.h"
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
                cqueue_push(queue, &value, -1);
        }

        return 0;
}

DWORD WINAPI poper(LPVOID lp)
{
        int i;
        char path[MAX_PATH];
        int value;
        int cnt = 5 * 1024 * 1024 / 4 / 5;

        sprintf(path, "C:\\Test\\%d%d.bin", 2, (unsigned long)lp);

        FILE *fp = fopen(path, "wb+");


        i = 0;
        while (cnt--) {
                cqueue_pop(queue, &value, -1);
                fwrite(&value, sizeof(value), 1, fp);
        }

	fclose(fp);
        printf("pop -->%d\n", i);

        return 0;
}

unsigned long cs_check()
{
	FILE *fp1;
	unsigned char value;
	unsigned long cs;
	char path[MAX_PATH];
	cs = 0;

	for (int i = 0; i < 10; i++) {
		sprintf(path, "C:\\Test\\2%d.bin", i + 1);
		fp1 = fopen(path, "rb");
		while (1) {
			if (fread(&value, sizeof(value), 1, fp1) != 1)
				break;
			cs += value;
		}

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

	/* 测试表明当队列大小不够时, 会导致读与写碰撞很厉害, 即使没有锁也是如此 */

        queue = cqueue_create(1024, sizeof(int));

        int cnt;

        cnt = 3;

        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);
        //cqueue_push(queue, &cnt);

	CACL_TAKES_TIME_BEGIN(aa);

        memset(h, 0, sizeof(h));
        h[0] = CreateThread(NULL, 0, pusher, (LPVOID)1, 0, NULL);
        h[1] = CreateThread(NULL, 0, pusher, (LPVOID)2, 0, NULL);
	for (int i = 0; i < 10; i++)
		h[i + 2] = CreateThread(NULL, 0, poper, (LPVOID)(i +1), 0, NULL);

        WaitForMultipleObjects(12, h, TRUE, -1);

	cs_check();

        printf("Test Finish, takes:%dms\n", CACL_TAKES_TIME_END(aa));
        system("PAUSE");


	return 0;
}


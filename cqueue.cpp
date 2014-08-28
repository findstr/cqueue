#include <Windows.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "cqueue.h"

struct cqueue {
        int elm_cnt;
        int elm_size;
        unsigned long head;
        unsigned long wtail;
        unsigned long tail;
        unsigned long rlock;
        unsigned char *buff;
};

#define  ELEM_FULL_CNT(tail, head, size) ((tail - head) & (size - 1))
#define  ELEM_EMPTY_CNT(tail, head, size) (size - ((tail- head) & (size - 1)) - 1)
#define  INDEX_ROUND(index, size)      ((index) & (size - 1))

static int __lock__(struct cqueue *lock)
{
        while(InterlockedCompareExchange(&lock->rlock, 1, 0) != 0)
                Sleep(0);

        return 0;
}

static int __unlock__(struct cqueue *lock)
{
        InterlockedExchange(&lock->rlock, 0);
        return 0;
}

static int to_power_2(int value)
{
        int i = 0;
        int shift = 1;
        for (i = 0; i < 32; i++) {
                if (shift >= value)
                        break;
                shift <<= 1;
        }

        return shift;
}

//(tail - head) & (size - 1) == element count

struct cqueue *cqueue_create(int cnt, int elm_size)
{
        struct cqueue *queue = (struct cqueue *)malloc(sizeof(*queue));
        if (queue == NULL)
                return queue;
 
        assert(cnt);
        assert(elm_size);

        memset(queue, 0, sizeof(*queue));
        queue->elm_cnt = to_power_2(cnt);
        queue->elm_size = elm_size;
        queue->buff = (unsigned char *)malloc(queue->elm_cnt * queue->elm_size);
        if (queue->buff == NULL) {
                free(queue);
                queue = NULL;
        }

        return queue;
}

int cqueue_push(struct cqueue *queue, const void *elm)
{
        unsigned long last_wtail;
        unsigned long wtail;
 
        assert(queue);
        assert(elm);

        do {
                wtail = queue->wtail;
                if (ELEM_EMPTY_CNT(wtail, queue->head, queue->elm_cnt) < 1) {
                        Sleep(0); /* other thread to pop the queue */
                        continue;
                }

                if (InterlockedCompareExchange(&queue->wtail, wtail + 1, wtail) == wtail)
                        break;
        } while (1);
 
        last_wtail = INDEX_ROUND(wtail, queue->elm_cnt);
        wtail++;
        wtail = INDEX_ROUND(wtail, queue->elm_cnt);

        memcpy(&queue->buff[wtail* queue->elm_size], elm, queue->elm_size);


        while (InterlockedCompareExchange(&queue->tail, wtail, last_wtail) != last_wtail)
                ;

        return 0;
}

int cqueue_pop(struct cqueue *queue, void *elm)
{
        unsigned long head;
        assert(queue);
        assert(elm);

        while ((void)0 , 1) {
                if(ELEM_FULL_CNT(queue->tail, queue->head, queue->elm_cnt) > 0) {
                        __lock__(queue);
                        if (ELEM_FULL_CNT(queue->tail, queue->head, queue->elm_cnt) > 0)
                                break;
                        else
                                __unlock__(queue);
                        Sleep(0);               /* other thread to push the queue */
                }
        }
        
        queue->head = INDEX_ROUND(queue->head + 1, queue->elm_cnt);
        head = queue->head;
        memcpy(elm, &queue->buff[head * queue->elm_size], queue->elm_size);

        __unlock__(queue);

        return 0;
}

int cqueue_free(struct cqueue *queue)
{
        assert(queue);
        assert(queue->buff);

        free(queue->buff);
        free(queue);

        return 0;
}
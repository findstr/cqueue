#ifndef _CQUEUE_H
#define _CQUEUE_H

struct cqueue;
typedef int (copyer_t)(void *dst, void *src);

struct cqueue *cqueue_create(int cnt, int elm_size);
int cqueue_push(struct cqueue *queue, const void *elm, int ms);
int cqueue_pop(struct cqueue *queue, void *elm, int ms);
int cqueue_free(struct cqueue *queue);



#endif // !_CQUEUE_H

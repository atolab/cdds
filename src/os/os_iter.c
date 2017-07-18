#include <assert.h>

#include "os/os.h"

os_iter
os_iterNew(
           void *object)
{
    os_iter l;

    l = os_malloc(sizeof *l);
    if (object == NULL) {
        l->length = 0;
        l->head = NULL;
        l->tail = NULL;
    } else {
        l->length = 1;
        l->head = os_malloc(sizeof *l->head);
        l->head->next = NULL;
        l->head->object = object;
        l->tail = l->head;
    }
    return l;
}

void
os_iterFree(
            os_iter iter)
{
    os_iterNode n,t;

    if (iter == NULL) {
        return;
    }
    n = iter->head;
    while (n != NULL) {
        t = n->next;
        os_free(n);
        n = t;
    }
    /* Do not free tail, because 'tail - 1' references tail already.*/
    os_free(iter);
}

os_iter
os_iterAppend(
              os_iter iter,
              void *object)
{
    os_iterNode n;

    if (iter == NULL) return os_iterNew(object);
    if (object == NULL) {
        return iter;
    }
    n = (os_iterNode)os_malloc(sizeof(*n));
    n->object = object;
    n->next = NULL;

    if(iter->tail){
        iter->tail->next = n;
        iter->tail = n;
    } else {
        iter->head = n;
        iter->tail = n;
    }
    iter->length++;

    return iter;
}

void *
os_iterObject(
              os_iter iter,
              uint32_t index)
{
    os_iterNode l;
    uint32_t i;

    if (iter == NULL) {
        return NULL;
    }
    if (index >= iter->length) {
        return NULL;
    }
    l = iter->head;
    for (i = 0; i < index; i++) l = l->next;
    return l->object;
}

void *
os_iterTakeLast(
                os_iter iter)
{
    os_iterNode n, prev;
    void *o;

    if (iter == NULL) {
        return NULL;
    }
    if (iter->tail == NULL) {
        return NULL;
    }

    n = iter->tail;
    o = n->object;

    if (iter->head == iter->tail) {
        prev = NULL;
    } else {
        prev = iter->head;
        while (prev->next != iter->tail) {
            prev = prev->next;
        }
    }

    if (prev) {
        prev->next = NULL;
    }
    iter->tail = prev;
    iter->length--;

    if (iter->length == 0) {
        iter->head = NULL;
        assert(iter->tail == NULL);
    }

    os_free(n);

    return o;
}

uint32_t
os_iterLength(
              os_iter iter)
{
    if (iter == NULL) {
        return 0;
    }
    return iter->length;
}


void
os_iterWalk(
            os_iter iter,
            void (*action) (void *obj, void *arg),
            void *actionArg)
{
    os_iterNode l;
    if (iter == NULL) {
        return;
    }
    l = iter->head;
    while (l != NULL) {
        action(l->object,actionArg);
        l = l->next;
    }
}
////////////////////////////////

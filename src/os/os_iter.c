#include <assert.h>
#include <string.h>

#include "os/os_iter.h"

typedef struct os_iterNode_s os_iterNode;

struct os_iterNode_s {
    os_iterNode *next;
    void *object;
};

struct os_iter_s {
    uint32_t length;
    os_iterNode *head;
    os_iterNode *tail;
};

_Ret_range_(-1, iter->length) static int32_t
os__iterIndex(
    _In_ const os_iter *iter,
    _In_ int32_t index)
{
    int32_t idx = 0;

    assert(iter != NULL);

    if (index == OS_ITER_LENGTH) {
        idx = iter->length;
    } else if (index < 0) {
        index *= -1;
        if ((uint32_t)index > iter->length) {
            idx = -1;
        } else {
            idx = iter->length - index;
        }
    } else {
        if ((uint32_t)index > iter->length) {
            idx = iter->length;
        } else {
            idx = index;
        }
    }

    return idx;
}

_Check_return_
_Ret_valid_
os_iter *
os_iterNew(
    void)
{
    os_iter *iter = os_malloc(sizeof *iter); /* TODO: replace with os_malloc_0 when on master */
    memset(iter, 0, sizeof *iter);

    return iter;
}

void
os_iterFree(
    _In_opt_ _Post_ptr_invalid_ os_iter *iter,
    _In_opt_ void(*func)(_Inout_ void *))
{
    os_iterNode *node, *next;

    if (iter != NULL) {
        node = iter->head;
        while (node != NULL) {
            next = node->next;
            if (node->object != NULL && func != NULL) {
                func(node->object);
            }
            os_free(node);
            node = next;
        }
        os_free(iter);
    }
}

_Success_(return >= 0) _Ret_range_(-1, INT32_MAX) int32_t
os_iterInsert(
    _Inout_ os_iter *iter,
    _In_opt_ void *object,
    _In_ int32_t index)
{
    int32_t cnt, idx = -1;
    os_iterNode *node, *prev;

    node = os_malloc(sizeof *node);
    memset(node, 0, sizeof(*node)); /* TODO: replace with os_malloc_0 on master */
    node->object = object;

    idx = os__iterIndex(iter, index);
    prev = NULL;
    if (idx > 0) {
        assert(iter->length != 0);
        assert(iter->head != iter->tail || iter->length == 1);
        if (idx == iter->length) {
            prev = iter->tail;
            iter->tail = node;
        } else {
            cnt = 1;
            prev = iter->head;
            while (cnt++ < idx) {
                prev = prev->next;
            }
            assert(prev != iter->tail);
            node->next = prev->next;
            prev->next = node;
        }
        prev->next = node;
    } else {
        assert(idx == 0 || idx == -1);
        idx = 0;
        node->next = iter->head;
        iter->head = node;
        if (iter->tail == NULL) {
            assert(iter->length == 0);
            iter->tail = node;
        }
    }

    iter->length++;

    return idx;
}

_Ret_opt_valid_ void *
os_iterObject(
    _In_ const os_iter *iter,
    _In_range_(INT32_MIN+1, INT32_MAX) int32_t index)
{
    os_iterNode *node;
    int32_t cnt, idx;
    void *obj = NULL;

    assert(iter != NULL);

    idx = os__iterIndex(iter, index);
    if (idx >= 0 && (uint32_t)idx < iter->length) {
        if (idx == (iter->length - 1)) {
            node = iter->tail;
        } else {
            node = iter->head;
            for (cnt = 0; cnt < idx; cnt++) {
                node = node->next;
            }
        }
        obj = node->object;
    }

    return obj;
}

_Ret_opt_valid_ void *
os_iterTake(
    _Inout_ os_iter *iter,
    _In_range_(INT32_MIN+1, INT32_MAX) int32_t index)
{
    os_iterNode *node, *prev;
    int32_t cnt, idx;
    void *obj = NULL;

    assert(iter != NULL);

    idx = os__iterIndex(iter, index);
    if (idx >= 0 && (uint32_t)idx < iter->length) {
        prev = NULL;
        node = iter->head;
        for (cnt = 0; cnt < idx; cnt++) {
            prev = node;
            node = node->next;
        }
        if (node == iter->head) {
            iter->head = node->next;
        } else {
            assert(prev != NULL);
            prev->next = node->next;
        }
        if (node == iter->tail) {
            iter->tail = prev;
        }

        obj = node->object;
        os_free(node);
        iter->length--;
    }

    return obj;
}

_Ret_range_(0, INT32_MAX) uint32_t
os_iterLength(
    _In_ const os_iter *__restrict iter)
{
    assert(iter != NULL);
    return iter->length;
}

void
os_iterWalk(
    _In_ const os_iter *iter,
    _In_ void(*func)(_Inout_ void *obj, _Inout_opt_ void *arg),
    _Inout_opt_ void *arg)
{
    os_iterNode *node;

    assert(iter != NULL);
    assert(func != NULL);

    node = iter->head;
    while (node != NULL) {
        if(node->object) {
            func(node->object, arg);
        }
        node = node->next;
    }
}

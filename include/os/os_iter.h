#ifndef OS_ITER_H
#define OS_ITER_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct os_iter_s *os_iter;
typedef struct os_iterNode_s *os_iterNode;

struct os_iter_s {
    uint32_t length;
    os_iterNode head;
    os_iterNode tail;
};

struct os_iterNode_s {
    os_iterNode next;
    void *object;
};

os_iter
os_iterNew(
    void *object);

void
os_iterFree(
    os_iter iter);

os_iter
os_iterAppend(
    os_iter iter,
    void *object);

void *
os_iterObject(
    os_iter iter,
    uint32_t index);

void *
os_iterTakeLast(
    os_iter iter);

uint32_t
os_iterLength(
    os_iter iter);

void
os_iterWalk(
    os_iter iter,
    void (*action) (void *obj, void *arg),
    void *actionArg);

#if defined (__cplusplus)
}
#endif

#endif /* OS_ITER_H */

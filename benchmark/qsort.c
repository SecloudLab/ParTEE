#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 50000000
#define MAX_THREADS 4

typedef struct {
    void *base;
    size_t n;
    size_t es;
    int (*cmp)(const void *, const void *);
} ThreadArgs;

static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static int active_threads = 0;

void qsort_parallel(void *base, size_t n, size_t es, int (*cmp)(const void *, const void *));

void *qsort_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    qsort_parallel(args->base, args->n, args->es, args->cmp);
    free(arg);

    pthread_mutex_lock(&thread_mutex);
    active_threads--;
    pthread_mutex_unlock(&thread_mutex);

    return NULL;
}

void qsort_parallel(void *base, size_t n, size_t es, int (*cmp)(const void *, const void *)) {
    char *a = (char *)base;
    if (n < 7) {
        // 使用插入排序处理小数组
        for (char *pm = a + es; pm < a + n * es; pm += es) {
            for (char *pl = pm; pl > a && cmp(pl - es, pl) > 0; pl -= es) {
                char tmp[es];
                memcpy(tmp, pl, es);
                memcpy(pl, pl - es, es);
                memcpy(pl - es, tmp, es);
            }
        }
        return;
    }

    // 基准选择
    char *pm = a + (n / 2) * es;
    char *pl = a;
    char *pn = a + (n - 1) * es;
    if (n > 7) {
        size_t d = (n / 8) * es;
        pl = pl + ((cmp(pl, pm) < 0) ? 0 : d);
        pn = pn - ((cmp(pn, pm) < 0) ? 0 : d);
    }
    pm = pm - ((cmp(pm, pn) < 0) ? es : 0);

    // 分区
    char *pa, *pb, *pc, *pd;
    pa = pb = a + es;
    pc = pd = a + (n - 1) * es;

    while (1) {
        while (pb <= pc && cmp(pb, a) <= 0) pb += es;
        while (pb <= pc && cmp(pc, a) >= 0) pc -= es;
        if (pb > pc) break;

        char tmp[es];
        memcpy(tmp, pb, es);
        memcpy(pb, pc, es);
        memcpy(pc, tmp, es);
        pb += es;
        pc -= es;
    }

    // 交换基准
    char tmp[es];
    memcpy(tmp, a, es);
    memcpy(a, pc, es);
    memcpy(pc, tmp, es);

    // 子区间递归
    size_t left_size = (pc - a) / es;
    size_t right_size = n - left_size - 1;

    pthread_t thread;
    int use_thread = 0;

    if (active_threads < MAX_THREADS) {
        pthread_mutex_lock(&thread_mutex);
        if (active_threads < MAX_THREADS) {
            active_threads++;
            use_thread = 1;
        }
        pthread_mutex_unlock(&thread_mutex);
    }

    if (use_thread) {
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->base = a + (left_size + 1) * es;
        args->n = right_size;
        args->es = es;
        args->cmp = cmp;
        pthread_create(&thread, NULL, qsort_thread, args);
    } else {
        qsort_parallel(a + (left_size + 1) * es, right_size, es, cmp);
    }

    qsort_parallel(a, left_size, es, cmp);

    if (use_thread) {
        pthread_join(thread, NULL);
    }
}

int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int main() {
    int *arr = malloc(ARRAY_SIZE * sizeof(int));
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand();
    }

    qsort_parallel(arr, ARRAY_SIZE, sizeof(int), compare);

    printf("Sorted: %d, %d\n", arr[0], arr[ARRAY_SIZE - 1]);

    free(arr);
    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define test(p) (primes[p >> 6] & 1 << (p & 0x3f))
#define set(p) (primes[p >> 6] |= 1 << (p & 0x3f))
#define is_prime(p) !test(p)

// 全局变量
int limit = 33333333;
uint64_t *primes; // 位图数组
pthread_mutex_t lock; // 锁，用于保护共享数据

// 线程参数
typedef struct {
    int start;
    int end;
    int sqrt_limit;
} ThreadData;

// 读取 RISC-V 的时间计数器
static inline uint64_t read_rdtime() {
    uint64_t time;
    asm volatile("rdtime %0" : "=r"(time)); // 使用 rdtime 指令
    return time;
}

// 筛选函数：每个线程筛选 start 到 end 的范围
void* sieve_range(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int start = data->start;
    int end = data->end;
    int sqrt_limit = data->sqrt_limit;

    for (int64_t p = 2; p <= sqrt_limit; p++) {
        if (is_prime(p)) {
            for (int64_t n = ((start + p - 1) / p) * p; n <= end; n += p) {
                pthread_mutex_lock(&lock);
                if (!test(n)) set(n);
                pthread_mutex_unlock(&lock);
            }
        }
    }
    return NULL;
}

int main() {
    // 初始化
    size_t primes_size = ((limit >> 6) + 1) * sizeof(uint64_t);
    primes = (uint64_t*)calloc(1, primes_size); // 位图初始化为0
    pthread_mutex_init(&lock, NULL);

    int sqrt_limit = (int)sqrt(limit);
    int thread_count = 4; // 线程数
    pthread_t threads[thread_count];
    ThreadData thread_data[thread_count];

    // 划分任务范围
    int range = (limit + thread_count - 1) / thread_count;
    for (int i = 0; i < thread_count; i++) {
        thread_data[i].start = i * range + 1;
        thread_data[i].end = (i + 1) * range;
        if (thread_data[i].end > limit) thread_data[i].end = limit;
        thread_data[i].sqrt_limit = sqrt_limit;
    }

    // 记录起始时间
    uint64_t start_time = read_rdtime();

    // 创建线程
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, sieve_range, &thread_data[i]);
    }

    // 等待所有线程完成
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // 记录结束时间
    uint64_t end_time = read_rdtime();

    // 找到最大的素数
    for (int i = limit; i > 0; i--) {
        if (is_prime(i)) {
            printf("Largest prime <= %d: %d\n", limit, i);
            break;
        }
    }

    // 输出计算时间
    printf("Computation time: %lu cycles\n", end_time - start_time);

    // 清理资源
    free(primes);
    pthread_mutex_destroy(&lock);

    return 0;
}

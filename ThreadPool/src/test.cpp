#include <threadpool/threadpool.hpp>
#include <vector>
#include <benchmark/benchmark.h>
#include <cmath>
#include <iostream>


constexpr size_t n = 1 << 24;
std::mutex ans_lock;

void test_calculation(float* ans,size_t s, size_t e) {
    float tmp = 0;
    for (size_t i = s; i < e; i++) {
        tmp += (std::sin(i)*std::cos(2*i) + 0.1);
    }
    std::lock_guard lgd(ans_lock);
    *ans += tmp;
}

void BM_normal(benchmark::State &bm) {
	for (auto _:bm) {
        float ans = 0;
        test_calculation(&ans, 0, n);
        benchmark::DoNotOptimize(ans);
        std::cout << "normal ans:" << ans << std::endl;
    }
}
BENCHMARK(BM_normal);

void BM_threadpool(benchmark::State &bm) {
    for (auto _:bm) {
        ThreadPool pool(6);
        pool.init();
        float ans = 0;
        for (int i = 0; i < (1<<13); i++) {
            pool.submit(test_calculation, &ans, i*2048, i*2048+2048);
        }
        pool.shutdown();
        std::cout << "threadpool ans:" << ans << std::endl;
        benchmark::DoNotOptimize(ans);
    }
}
BENCHMARK(BM_threadpool);

BENCHMARK_MAIN();
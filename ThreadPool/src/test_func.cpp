// test the correctness of the threadpool

#include <iostream>
#include <random>
#include <threadpool/threadpool.hpp>
#include <chrono>
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-1000, 1000);
auto rnd = std::bind(dist, mt);

//模拟长时间工作
void simulate_hard_computation()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
}

//无返回参数
void multiply(const int a, const int b)
{
	simulate_hard_computation();
	const int res = a * b;
	std::cout << a << " * " << b << " = " << res << std::endl;
}

//out型返回
void multiply_output(int& out, const int a, const int b)
{
	simulate_hard_computation();
	out = a * b;
	std::cout << a << " * " << b << " = " << out << std::endl;
}

//直接返回
int multiply_return(const int a, const int b)
{
	simulate_hard_computation();
	const int res = a * b;
	std::cout << a << " * " << b << " = " << res << std::endl;
	return res;
}

//样例测试函数
void example()
{
	ThreadPool pool(3);
	pool.init();
	for (int i = 1; i <= 3; ++i)
		for (int j = 1; j <= 10; ++j)
		{
			pool.submit(multiply, i, j);
		}
	int output_ref;
	auto future1 = pool.submit(multiply_output, std::ref(output_ref), 5, 6);
    future1.get();
	std::cout << "Last operation result is equals to " << output_ref << std::endl;
	auto future2 = pool.submit(multiply_return, 5, 3);
	int res = future2.get();
	std::cout << "Last operation result is equals to " << res << std::endl;
	pool.shutdown();
}
int main()
{
	example();
	return 0;
}
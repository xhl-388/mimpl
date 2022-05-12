#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <future>
#include <functional>
#include <iostream>

/// <summary>
/// 线程安全的队列
/// </summary>
/// <typeparam name="T">元素类型</typeparam>
template <typename T>
class SynchronizedQueue
{
private:
	std::queue<T> m_queue; //容器
	std::mutex m_mutex;		//容器访问锁
public:
	SynchronizedQueue() {}
	SynchronizedQueue(const SynchronizedQueue& sq) = delete;
	SynchronizedQueue(SynchronizedQueue&& other) = delete;

	bool empty()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.empty();
	}

	size_t size()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.size();
	}

	//t是一个out参数
	bool pop(T &t)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_queue.empty())
			return false;
		t = std::move(m_queue.front());
		m_queue.pop();
		return true;
	}

	void push(const T& t)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_queue.emplace(t);
	}
};

/// <summary>
/// 线程池管理类
/// </summary>
class ThreadPool
{
private:
	/// <summary>
	/// 真正的工作者
	/// </summary>
	class ThreadWorker
	{
	private:
		int m_id;	//工人id
		ThreadPool* m_pool;	//所属池
	public:
		ThreadWorker(ThreadPool* pool,const int id):m_pool(pool),m_id(id)
		{
		}

		//类函数调用
		void operator()()
		{
			std::function<void()> func;
			bool dequeued;
			while (!m_pool->m_shutdown || !m_pool->m_task_queue.empty())
			{
				//加一层以即使释放mutex，避免func阻塞其它线程
				{
					std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
					if (m_pool->m_task_queue.empty())
						m_pool->m_conditional_lock.wait(lock);	//等待队列加入新的元素唤醒该工作者
					dequeued = m_pool->m_task_queue.pop(func);
				}
				if (dequeued)
					func();		//处理任务
			}
		}
	};
	bool m_shutdown;	//线程池是否关闭
	SynchronizedQueue<std::function<void()>> m_task_queue;	//任务队列
	std::vector<std::future<void>> m_threads;		//线程池所管理的线程
	std::mutex m_conditional_mutex;		//访问任务队列的互斥锁
	std::condition_variable m_conditional_lock;		//提交函数与工人的同步
public:
	ThreadPool(const int n_threads = 4) :
		m_threads(std::vector<std::future<void>>(n_threads)), m_shutdown(false)
	{

	}
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&) = delete;

	//初始化线程池
	void init()
	{
		for (int i = 0; i < m_threads.size(); i++) {
			m_threads.at(i) = std::async(std::launch::async, ThreadWorker(this, i));
		}
	}	

	//关闭线程池
	void shutdown()
	{
		m_shutdown = true;
		m_conditional_lock.notify_all();
		for (int i = 0; i < m_threads.size(); i++) {
			m_threads.at(i).wait();
		}
	}

	//任务提交函数
	template<typename Func,typename... Args>
	auto submit(Func&& f, Args&& ...args)->std::future<decltype(f(args...))>
	{
		std::function<decltype(f(args...))()> func = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
		auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
		std::function<void()> wrapped_func = [task_ptr] {(*task_ptr)(); };
		m_task_queue.push(wrapped_func);
		m_conditional_lock.notify_one();	//唤醒一个阻塞的工人
		return task_ptr->get_future();	
	}
};
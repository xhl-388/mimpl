#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <future>
#include <functional>
#include <iostream>

/// <summary>
/// �̰߳�ȫ�Ķ���
/// </summary>
/// <typeparam name="T">Ԫ������</typeparam>
template <typename T>
class SynchronizedQueue
{
private:
	std::queue<T> m_queue; //����
	std::mutex m_mutex;		//����������
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

	//t��һ��out����
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
/// �̳߳ع�����
/// </summary>
class ThreadPool
{
private:
	/// <summary>
	/// �����Ĺ�����
	/// </summary>
	class ThreadWorker
	{
	private:
		int m_id;	//����id
		ThreadPool* m_pool;	//������
	public:
		ThreadWorker(ThreadPool* pool,const int id):m_pool(pool),m_id(id)
		{
		}

		//�ຯ������
		void operator()()
		{
			std::function<void()> func;
			bool dequeued;
			while (!m_pool->m_shutdown || !m_pool->m_task_queue.empty())
			{
				//��һ���Լ�ʹ�ͷ�mutex������func���������߳�
				{
					std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
					if (m_pool->m_task_queue.empty())
						m_pool->m_conditional_lock.wait(lock);	//�ȴ����м����µ�Ԫ�ػ��Ѹù�����
					dequeued = m_pool->m_task_queue.pop(func);
				}
				if (dequeued)
					func();		//��������
			}
		}
	};
	bool m_shutdown;	//�̳߳��Ƿ�ر�
	SynchronizedQueue<std::function<void()>> m_task_queue;	//�������
	std::vector<std::future<void>> m_threads;		//�̳߳���������߳�
	std::mutex m_conditional_mutex;		//����������еĻ�����
	std::condition_variable m_conditional_lock;		//�ύ�����빤�˵�ͬ��
public:
	ThreadPool(const int n_threads = 4) :
		m_threads(std::vector<std::future<void>>(n_threads)), m_shutdown(false)
	{

	}
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&) = delete;

	//��ʼ���̳߳�
	void init()
	{
		for (int i = 0; i < m_threads.size(); i++) {
			m_threads.at(i) = std::async(std::launch::async, ThreadWorker(this, i));
		}
	}	

	//�ر��̳߳�
	void shutdown()
	{
		m_shutdown = true;
		m_conditional_lock.notify_all();
		for (int i = 0; i < m_threads.size(); i++) {
			m_threads.at(i).wait();
		}
	}

	//�����ύ����
	template<typename Func,typename... Args>
	auto submit(Func&& f, Args&& ...args)->std::future<decltype(f(args...))>
	{
		std::function<decltype(f(args...))()> func = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
		auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
		std::function<void()> wrapped_func = [task_ptr] {(*task_ptr)(); };
		m_task_queue.push(wrapped_func);
		m_conditional_lock.notify_one();	//����һ�������Ĺ���
		return task_ptr->get_future();	
	}
};
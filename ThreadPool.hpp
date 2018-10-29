/*******************************************************************************
* Copyright 2018-2019 Juan Francisco Crespo Galán
*
* Licensed under the MIT License, you may not use this file except in compliance with the License.
* https://opensource.org/licenses/MIT
*
*******************************************************************************/

#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <functional>
#include <memory>
#include <vector>
#include <queue>


/**
* This class represents a generic pool of threads that are able to perform user-defined tasks.
*/
class ThreadPool
{
private:
	size_t m_numthreads;
	std::vector<std::thread> m_threads;
	std::mutex m_mutex;
	std::condition_variable m_condition;

	/**
	* A queue of tasks.
	*/
	std::queue<std::function<void()>> m_queue;

	/**
	* Stop flag. Signals all threads to stop when they finish their current task
	*/
	bool m_stopflag;

	/**
	* Soft stop flag. Signals all threads to stop when they finish all enqueued tasks
	*/
	bool m_stopflag_soft;

public:
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	/**
	* Creates and initialices a ThreadPool with the specified number of threads.
	* \param count The desired number of threads
	*/
	ThreadPool(size_t numthreads) :
		m_stopflag{ false }, m_stopflag_soft{ false }, m_numthreads{ numthreads }
	{
		init(numthreads);
	}

	/**
	* Waits until the task queue is cleared before destroying the object. If you don't want this, call stop() before destruction.
	*/
	virtual ~ThreadPool() 
	{
		softStop();
	}

	/**
	* Initializes a stopped threadpool with a set number of threads.
	* \param count The desired number of threads
	* \return true if the threadpool was initialized correctly, false if it still needs to be stopped.
	*/
	bool init(size_t numthreads)
	{
		if (m_threads.size() != 0)
		{
			return false;
		}

		m_stopflag = false;
		m_stopflag_soft = false;
		m_numthreads = numthreads;
		m_threads.reserve(numthreads);
		for (auto i = 0u; i < numthreads; i++)
		{
			m_threads.emplace_back(&ThreadPool::threadFunction, this);
		}
		return true;
	}

	/**
	* Stops the threadpool in an orderly manner and removes all threads so it can be resized with a call to init().
	* Bear in mind that tasks can be left enqueued.
	*/
	void stop()
	{
		{
			auto&& guard{ std::lock_guard{m_mutex} };
			m_stopflag = true;
		}
		m_condition.notify_all();
		clearThreads();
	}

	/**
	* Stops the threadpool in an orderly manner and removes all threads so it can be resized with a call to init().
	* All tasks in the queue are executed before stopping.
	*/
	void softStop()
	{
		{
			auto&& guard{ std::lock_guard{m_mutex} };
			m_stopflag_soft = true;
		}
		m_condition.notify_all();
		clearThreads();
	}

	/**
	* Enqueues a new task for the threads to execute.
	* \param t A function that realices a task.
	* \param args The arguments of the task.
	* \return A future of the same type as the return type of the task.
	*/
	template<class T, class... Args>
	auto enqueue(T&& t, Args&&... args)
	{
		using pkgdTask = std::packaged_task<decltype(t(args...))()>;

		auto task{ std::make_shared<pkgdTask>(std::bind(std::forward<T>(t), std::forward<Args>(args)...)) };
		auto future{ task->get_future() };
		{
			auto&& guard{ std::lock_guard{m_mutex} };
			m_queue.emplace([tsk = std::move(task)]() { (*tsk)(); });
		}
		m_condition.notify_one();
		return future;
	}

	/**
	* Retrieves the number of threads.
	* \return The number of threads
	*/
	size_t numThreads() const noexcept
	{
		return m_numthreads;
	}

private:
	void threadFunction()
	{
		for (;;)
		{
			std::function<void()> task;
			{
				auto lock{ std::unique_lock{ m_mutex } };
				m_condition.wait(lock, [this] { return m_stopflag_soft || m_stopflag || !m_queue.empty(); });
				if ((m_stopflag_soft && m_queue.empty()) || m_stopflag)
				{
					return;
				}
				task = m_queue.front();
				m_queue.pop();
			}
			task();
		}
	}

	void clearThreads()
	{
		for (auto& thread : m_threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
		m_threads.clear();
	}
};

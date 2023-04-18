#pragma once

#include <mutex>
#include <queue>
#include <functional>
#include <vector>

#include "Promise.h"

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

	void stop();

	template <typename T>
	Promise<T> runAsync(const std::function<T()>& func) {
		Promise<T> resultPromise;

		const std::function<void()>& function_on_queue = [func, resultPromise]() mutable {
			const T& result = func();
			resultPromise.setValue(result);
		};

		std::lock_guard<std::mutex> guard(mutex_);
		if (!continue_) {
			return Promise<T>();
		}

		task_queue_.push_back(function_on_queue);

		return resultPromise;
	}

private:
	static void function_on_thread(std::mutex&, std::deque<std::function<void()>>&, const bool&);

	const size_t thread_count_;
	std::vector<std::thread> threads_;
	
	std::mutex mutex_;
	std::deque<std::function<void()>> task_queue_;
	bool continue_;

};

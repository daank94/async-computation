#include "../include/ThreadPool.h"

#include <chrono>

ThreadPool::ThreadPool()
	: thread_count_(8), continue_(true)
{
	// Initialize the threads
	for (size_t i = 0; i < thread_count_; i++) {
		threads_.push_back(
			std::thread(function_on_thread, std::ref(mutex_), std::ref(task_queue_), std::ref(continue_))
		);
	}
}

ThreadPool::~ThreadPool() {
	stop();
}

void ThreadPool::stop() {
	{
		std::lock_guard<std::mutex> guard(mutex_);

		if (!continue_) {
			return;
		}

		continue_ = false;
	}

	bool queue_empty = false;
	while (!queue_empty) {
		{
			std::lock_guard<std::mutex> guard(mutex_);
			queue_empty = task_queue_.empty();
		}

		if (!queue_empty) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}

	// Threads should now be stopped, join them.
	for (std::thread& thread : threads_) {
		thread.join();
	}
}

void ThreadPool::function_on_thread(std::mutex& mutex_, std::deque<std::function<void()>>& task_queue, const bool& should_continue) {
	while (true) {
		std::function<void()> task_from_queue;
		{
			std::lock_guard<std::mutex> guard(mutex_);

			if (task_queue.empty()) {
				if (!should_continue) {
					break;
				}

				continue;
			}

			task_from_queue = task_queue.front();
			task_queue.pop_front();
		}
		// Mutex is unlocked here to allow other threads to access the queue.
		// Perform the task that was taken from the queue.
		task_from_queue();
	}
}

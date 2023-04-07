#pragma once

#include <chrono>
#include <mutex>
#include <thread>
#include <memory>

/**
 * Representing a value that will be calculated in the future.
 */
template <typename T>
class Promise : public std::enable_shared_from_this<Promise<T>>
{
public:
	std::shared_ptr<Promise<T>> get_ptr() {
		return this->shared_from_this();
	}

	static std::shared_ptr<Promise<T>> create() {
		// Not using std::make_shared<Best> because the c'tor is private.
		return std::shared_ptr<Promise>(new Promise());
	}

	/**
	 * Retrieves the promised value. Will block until the value is available.
	 */
	T getValue() {
		bool ready = false;

		// This can also be done smarter, using thread signalling instead of sleeping.
		while (!ready) {
			{
				const std::lock_guard<std::mutex> guard(mutex_);
				ready = isReady();
			}

			if (!ready) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
		}

		return *value_;
	}

	/**
	 * Sets the promised value which is the result of a calculation happening somehwere else.
	 */
	void setValue(const T& value) {
		const std::lock_guard<std::mutex> guard(mutex_);

		value_ = std::make_unique<T>(value);
	}

	Promise(const Promise&) = delete;
	Promise& operator=(const Promise&) = delete;
	Promise(Promise&&) = delete;
	Promise& operator=(Promise&&) = delete;

private:
	Promise() = default;

	/**
	 * Checks if the result is ready. This function does not lock the mutex, which must be done by the caller.
	 */
	bool isReady() {
		return static_cast<bool>(value_);
	}

	std::unique_ptr<T> value_;

	std::mutex mutex_;
};

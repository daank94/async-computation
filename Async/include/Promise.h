#pragma once

#include <chrono>
#include <mutex>
#include <thread>
#include <memory>

/**
 * Representing a value that will be calculated in the future.
 */
template <typename T>
class Promise
{
	template <typename T>
	class Impl;

public:

	Promise()
		: impl_(std::make_shared<Promise::Impl<T>>())
	{}

	T getValue() {
		return impl_->getValue();
	}

	void setValue(const T& value) {
		impl_->setValue(value);
	}

private:

	std::shared_ptr<Impl<T>> impl_;

	template <>
	class Impl<T> {
	public:
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

		Impl() = default;

		Impl(const Impl&) = delete;
		Impl& operator=(const Impl&) = delete;
		Impl(Impl&&) = delete;
		Impl& operator=(Impl&&) = delete;

	private:
		/**
		 * Checks if the result is ready. This function does not lock the mutex, which must be done by the caller.
		 */
		bool isReady() {
			return static_cast<bool>(value_);
		}

		std::unique_ptr<T> value_;
		std::mutex mutex_;
	};
};

#pragma once

#include <chrono>
#include <mutex>
#include <thread>
#include <memory>
#include <condition_variable>
#include <optional>

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
			std::unique_lock<std::mutex> lock(mutex_);
			cv_.wait(lock, [this]{ return this->value_optional_.has_value(); });

			return value_optional_.value();
		}

		/**
		 * Sets the promised value which is the result of a calculation happening somehwere else.
		 */
		void setValue(const T& value) {
			{
				std::lock_guard<std::mutex> guard(mutex_);
				value_optional_ = value;
			}
			cv_.notify_all();
		}

		Impl() = default;

		Impl(const Impl&) = delete;
		Impl& operator=(const Impl&) = delete;
		Impl(Impl&&) = delete;
		Impl& operator=(Impl&&) = delete;

	private:
		std::mutex mutex_;
		std::condition_variable cv_;
		std::optional<T> value_optional_;
	};
};

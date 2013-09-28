#ifndef __CONTAINER_BOUNDED_BLOCKING_QUEUE_HPP
#define __CONTAINER_BOUNDED_BLOCKING_QUEUE_HPP

#include <mutex>
#include <queue>
#include <chrono>


/*
限制大小的阻塞队列，适用于生产者消费者

bounded_block_queue_t



*/

namespace stdex { namespace container {

		template< typename T, typename A = std::allocator<T> >
		class bounded_block_queue_t
		{
			typedef std::lock_guard<std::mutex> auto_lock;

			mutable std::mutex mutex_;
			std::condition_variable not_empty_;
			std::condition_variable not_full_;
			std::deque<T, A> queue_;

			std::uint32_t max_size_;

		public:
			explicit bounded_block_queue_t(std::uint32_t maxSize)
				: max_size_(maxSize)
			{} 
			bounded_block_queue_t(std::uint32_t maxSize, A &allocator)
				: max_size_(maxSize)
				, queue_(allocator)
			{}

		private:
			bounded_block_queue_t(const bounded_block_queue_t &);
			bounded_block_queue_t &operator=(const bounded_block_queue_t &);

		public:
			void put(T &&x, std::chrono::milliseconds time_out = INFINITE)
			{
				{
					auto_lock lock(mutex_);
					while(queue_.size() == max_size_ )
						not_full_.wait_for(mutex_, time_out);

					assert(queue_.size() != max_size_);
					queue_.push_back(std::forward<T>(x));
				}

				not_empty_.notify_one();
			}

			T get()
			{
				T front;

				{
					auto_lock lock(mutex_);
					while(queue_.empty())
						not_empty_.wait(mutex_, std::chrono::milliseconds(INFINITE));

					assert(!queue_.empty());
					front = std::move(queue_.front());
					queue_.pop_front();
				}

				not_full_.notify_one();
				return front;
			}

			bool empty() const
			{
				auto_lock lock(mutex_);
				return queue_.empty();
			}

			bool full() const
			{
				auto_lock lock(mutex_);
				return queue_.size() == max_size_;
			}

			size_t size() const
			{
				auto_lock lock(mutex_);
				return queue_.size();
			}
		};


	}
}

#endif  


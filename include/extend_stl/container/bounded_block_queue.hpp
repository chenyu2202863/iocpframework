#ifndef __CONTAINER_BOUNDED_BLOCKING_QUEUE_HPP
#define __CONTAINER_BOUNDED_BLOCKING_QUEUE_HPP


#include "../../multi_thread/lock.hpp"
#include <queue>



/*
限制大小的阻塞队列，适用于生产者消费者

bounded_block_queue_t



*/

namespace stdex
{


	namespace container
	{

		template< typename T, typename A = std::allocator<T> >
		class bounded_block_queue_t
		{
			typedef multi_thread::critical_section		Mutex;
			typedef multi_thread::auto_lock_t<Mutex>	AutoLock;
			typedef multi_thread::semaphore_condition	Condtion;
			typedef std::deque<T, A>					Container;

			mutable Mutex mutex_;
			Condtion not_empty_;
			Condtion not_full_;
			Container queue_;

			size_t max_size_;

		public:
			explicit bounded_block_queue_t(size_t maxSize)
				: max_size_(maxSize)
			{} 
			bounded_block_queue_t(size_t maxSize, A &allocator)
				: max_size_(maxSize)
				, queue_(allocator)
			{}

		private:
			bounded_block_queue_t(const bounded_block_queue_t &);
			bounded_block_queue_t &operator=(const bounded_block_queue_t &);

		public:
			void put(const T &x, DWORD time_out = INFINITE)
			{
				{
					AutoLock lock(mutex_);
					while(queue_.size() == max_size_ )
						not_full_.wait(mutex_, time_out);

					assert(queue_.size() != max_size_);
					queue_.push_back(x);
				}

				not_empty_.signal();
			}

			T get()
			{
				T front;

				{
					AutoLock lock(mutex_);
					while(queue_.empty())
						not_empty_.wait(mutex_, INFINITE);

					assert(!queue_.empty());
					front = queue_.front();
					queue_.pop_front();
				}

				not_full_.signal();
				return front;
			}

			bool empty() const
			{
				AutoLock lock(mutex_);
				return queue_.empty();
			}

			bool full() const
			{
				AutoLock lock(mutex_);
				return queue_.size() == max_size_;
			}

			size_t size() const
			{
				AutoLock lock(mutex_);
				return queue_.size();
			}
		};


	}
}

#endif  


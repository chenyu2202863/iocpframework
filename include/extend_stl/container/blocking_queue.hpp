#ifndef __CONTAINER_BOUNDED_QUEUE_HPP
#define __CONTAINER_BOUNDED_QUEUE_HPP


#include "../../multi_thread/lock.hpp"
#include <queue>

/*
阻塞队列，适用于生产者消费者

	block_queue_t

*/
namespace stdex
{
	namespace container
	{

		template< typename T, typename A = std::allocator<T> >
		class blocking_queue_t
		{
			typedef multi_thread::critical_section		Mutex;
			typedef multi_thread::auto_lock_t<Mutex>	AutoLock;
			typedef multi_thread::semaphore_condition	Condtion;
			typedef std::deque<T, A>					Container;

			mutable Mutex mutex_;
			Condtion not_empty_;
			Container queue_;

		public:
			blocking_queue_t()
			{} 
			explicit blocking_queue_t(A &allocator)
				: queue_(allocator)
			{}

		private:
			blocking_queue_t(const blocking_queue_t &);
			blocking_queue_t &operator=(const blocking_queue_t &);

		public:
			void put(const T &x)
			{
				{
					AutoLock lock(mutex_);
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
					{
						not_empty_.wait(mutex_);
					}
					assert(!queue_.empty());
					front = queue_.front();
					queue_.pop_front();
				}

				return front;
			}

			size_t size() const
			{
				AutoLock lock(mutex_);
				return queue_.size();
			}

			bool empty() const
			{
				AutoLock lock(mutex_);
				return queue_.empty();
			}

			template < typename FuncT >
			void for_each(const FuncT &func)
			{
				AutoLock lock(mutex_);
				std::for_each(queue_.begin(), queue_.end(), func);
			}
		};

	}

}

#endif  


#ifndef __CONTAINER_BOUNDED_QUEUE_HPP
#define __CONTAINER_BOUNDED_QUEUE_HPP


/** @blocking_queue.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/08>
* @version <0.1>
*
* 生产者消费者容器
*/


#include <mutex>
#include <queue>
#include <condition_variable>

/*
阻塞队列，适用于生产者消费者

	block_queue_t

*/
namespace stdex
{
	namespace container
	{
		/**
		* @class <sync_sequence_container_t>
		* @brief 生产者消费者容器，接口与stl容器类似，采用FIFO算法
		*
		* T 值类型
		* A 内存分配器，在高性能的地方需要自己提供内存分配器
		*/

		template< typename T, typename A = std::allocator<T> >
		class blocking_queue_t
		{
			typedef std::mutex				Mutex;
			typedef std::unique_lock<Mutex>	AutoLock;
			typedef std::condition_variable	Condtion;
			typedef std::deque<T, A>		Container;

			mutable Mutex mutex_;
			Condtion not_empty_;
			Container queue_;

		public:
			blocking_queue_t()
			{} 

			/**
			* @brief 传入一个allocator
			* @param <alloc> <allocator对象>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <无>
			* @remarks <提高内存分配效率>
			*/
			explicit blocking_queue_t(A &allocator)
				: queue_(allocator)
			{}

		private:
			blocking_queue_t(const blocking_queue_t &);
			blocking_queue_t &operator=(const blocking_queue_t &);

		public:
			/**
			* @brief 把数据压入队列，生产一个数据
			* @param <x> <压入数据>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <线程安全，可并发多次调用>
			* @remarks <无>
			*/
			void put(T &&x)
			{
				{
					AutoLock lock(mutex_);
					queue_.push_back(std::move(x));
				}

				not_empty_.notify_one();
			}

			/**
			* @brief 把数据弹出队列，消费一个数据
			* @param <无>
			* @exception <不会抛出任何异常>
			* @return <弹出一个数据>
			* @note <线程安全，可并发多次调用>
			* @remarks <无>
			*/
			T get()
			{
				T front;
				{
					AutoLock lock(mutex_);
					while(queue_.empty())
					{
						not_empty_.wait(lock);
					}
					assert(!queue_.empty());
					front = std::move(queue_.front());
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

			/**
			* @brief 遍历队列
			* @param <func> <func调用约定为void(const T &val)>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <无>
			* @remarks <无>
			*/
			template < typename FuncT >
			void for_each(const FuncT &func)
			{
				AutoLock lock(mutex_);
				std::for_each(queue_.begin(), queue_.end(), func);
			}

			void clear()
			{
				AutoLock lock(mutex_);
				queue_.clear();
			}
		};

	}

}

#endif  


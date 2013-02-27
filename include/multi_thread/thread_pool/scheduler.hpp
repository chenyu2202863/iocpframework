#ifndef __MULTI_THREAD_THREAD_POOL_SCHEDULE_HPP
#define __MULTI_THREAD_THREAD_POOL_SCHEDULE_HPP


#include <deque>
#include <queue>

#include "task_adaptor.hpp"
#include "../../multi_thread/lock.hpp"


namespace multi_thread
{
	namespace threadpool
	{

		template < typename ContainerT >
		struct scheduler_base_t
		{	
			typedef multi_thread::critical_section		Mutex;
			typedef multi_thread::auto_lock_t<Mutex>	AutoLock;
			typedef multi_thread::semaphore_condition	Condtion;
			typedef ContainerT							Container;
			typedef typename Container::value_type		task_type;

			mutable Mutex mutex_;
			Condtion not_empty_;
			Container queue_;


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

			void clear()
			{
				AutoLock lock(mutex_);
				return queue_.clear();
			}
		};

		// ----------------------------------------
		// 先进先出调度

		template < typename TaskT = task_function >
		class fifo_scheduler_t
			: public scheduler_base_t<std::deque<TaskT>>
		{
		public:
			void push(const task_type &task)
			{
				{
					AutoLock lock(mutex_);
					queue_.push_back(task);
				}

				not_empty_.signal();
			}

			task_type pop()
			{
				AutoLock lock(mutex_);

				while(queue_.empty())
				{
					not_empty_.wait(mutex_);
				}
					
				assert(!queue_.empty());

				task_type val = queue_.front();
				queue_.pop_front();

				return val;
			}
		};



		// ----------------------------------------
		// 后进先出调度

		template < typename TaskT = task_function >
		class lifo_scheduler_t
			: public scheduler_base_t<std::deque<TaskT>> 
		{
		public:
			bool push(const task_type &task)
			{
				{
					AutoLock lock(mutex_);
					queue_.push_front(task);
				}

				not_empty_.signal();
			}

			task_type pop()
			{
				AutoLock lock(mutex_);

				while(queue_.empty())
				{
					not_empty_.wait(mutex_);
				}

				assert(!queue_.empty());
			
				task_type val = queue_.front();
				queue_.pop_front();

				return val;
			}
		};


		// ----------------------------------------
		// 优先级调度

		template < typename TaskT = task_function >
		class prio_scheduler_t
			: public scheduler_base_t<std::priority_queue<TaskT>>
		{
		public:
			bool push(const task_type &task)
			{
				{
					AutoLock lock(mutex_);
					queue_.push(task);
				}

				not_empty_.signal();
			}

			task_type pop()
			{
				AutoLock lock(mutex_);

				while(queue_.empty())
				{
					not_empty_.wait(mutex_);
				}

				assert(!queue_.empty());

				task_type val = queue_.front();
				queue_.pop_front();

				return val;
			}


			void clear()
			{
				AutoLock lock(mutex_);
				while( !empty() )
					pop();
			}
		};
	}
}





#endif
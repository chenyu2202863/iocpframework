#ifndef __MULTI_THREAD_THREAD_POOL_HPP
#define __MULTI_THREAD_THREAD_POOL_HPP

#include "detail/impl.hpp"
#include "task_adaptor.hpp"
#include "scheduler.hpp"
#include "size_controller.hpp"


/*
不同类型线程池

	static_fifo_pool
	staticL_lifo_pool
	static_prio_pool

	dynamic_fifo_pool
	dynamic_lifo_pool
	dynamic_prio_pool

	static_fifo_pool-->thread_pool

*/

namespace multi_thread
{
	namespace threadpool
	{

		// ---------------------------------------------------

		template < 
			typename TaskT									= task_function,
			template < typename > class ScheduleT			= fifo_scheduler_t,
			template < typename > class SizeCtlT			= static_size
		>
		class thread_pool_t
		{	
			typedef detail::thread_pool_impl_t<TaskT, ScheduleT, SizeCtlT>	impl_type;

		public:
			typedef TaskT								task_type;
			typedef ScheduleT<task_type>				scheduler_type;
			
		private:
			std::auto_ptr<impl_type> impl_;

		public:
			explicit thread_pool_t(size_t cnt = 0)
				: impl_(new impl_type)
			{
				impl_->resize(cnt == 0 ? 4 : cnt);
			}
			~thread_pool_t()
			{
				shutdown();
			}

		private:
			thread_pool_t(const thread_pool_t &);
			thread_pool_t &operator=(const thread_pool_t &);

		public:
			size_t size() const
			{
				return impl_->size();
			}

			size_t active_size() const
			{
				return impl_->active_size();
			}

			void call(const task_type &task)
			{
				impl_->schedule(task);
			}

			void clear()
			{
				impl_->clear();
			}

			bool empty() const
			{
				return impl_->empty();
			}

			void shutdown()
			{
				impl_->shutdown();
			}
		};


		typedef thread_pool_t<task_function, fifo_scheduler_t, static_size>		static_fifo_pool;
		typedef thread_pool_t<task_function, lifo_scheduler_t, static_size>		staticL_lifo_pool;
		typedef thread_pool_t<prio_task_function, prio_scheduler_t, static_size>	static_prio_pool;

		typedef thread_pool_t<task_function, fifo_scheduler_t, dynamic_size>		dynamic_fifo_pool;
		typedef thread_pool_t<task_function, lifo_scheduler_t, dynamic_size>		dynamic_lifo_pool;
		typedef thread_pool_t<prio_task_function, prio_scheduler_t, dynamic_size>	dynamic_prio_pool;

		typedef static_fifo_pool thread_pool; 
	}
}






#endif
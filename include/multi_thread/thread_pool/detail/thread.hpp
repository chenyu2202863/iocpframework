#ifndef __MULTI_THREAD_THREAD_POOL_THREAD_HPP
#define __MULTI_THREAD_THREAD_POOL_THREAD_HPP


#include <memory>

#include "../../lock.hpp"
#include "../../thread.hpp"


namespace multi_thread
{
	namespace threadpool
	{
		namespace detail
		{


			// --------------------------------------
			// 工作线程

			template < typename PoolT >
			class worker_thread_t
			{
			public:
				typedef PoolT							PoolType;
				typedef multi_thread::thread_impl_ex	ThreadType;

			private:
				PoolT *pool_;					// 指向宿主
				ThreadType thread_;				// thread which run loop

			public:
				explicit worker_thread_t(PoolT *pool)
					: pool_(pool)
				{
				}
				~worker_thread_t()
				{

				}

			public:
				DWORD run()
				{
					while(pool_->exceute_task())
					{}
					
					return 0;
				}

				
				void join()
				{
					thread_.stop();
				}

			public:
				// 静态工厂
				static std::tr1::shared_ptr<worker_thread_t> create(PoolT *pool)
				{
					std::tr1::shared_ptr<worker_thread_t> worker(new worker_thread_t(pool));
					
					worker->thread_.register_callback(std::tr1::bind(&worker_thread_t::run, worker.get()));
					worker->thread_.start();

					return worker;
				}
			};



		}
	}
}


#endif
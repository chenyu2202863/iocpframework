#ifndef __MULTI_THREAD_THREAD_POOL_POOL_ADAPTORS_HPP
#define __MULTI_THREAD_THREAD_POOL_POOL_ADAPTORS_HPP


#include <functional>
#include <memory>

namespace multi_thread
{
	namespace threadpool
	{
		// pool->schedule(bind(&Runnable::run, task_object)).
		
		template < typename PoolT, typename RunnableT >
		inline bool call(PoolT &pool, const std::tr1::shared_ptr<RunnableT> &obj)
		{
			return pool.call(std::tr1::bind(&RunnableT::Run, obj));
		}

		template < typename PoolT, typename RunnableT >
		inline bool call(PoolT &pool, const RunnableT &obj)
		{
			return pool.call(std::tr1::bind(&RunnableT::Run, std::tr1::ref(obj)));
		}

	}
}





#endif
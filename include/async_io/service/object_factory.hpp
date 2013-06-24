#ifndef __ASYNC_SERVICE_OBJECT_FACTORY_HPP
#define __ASYNC_SERVICE_OBJECT_FACTORY_HPP

#include <memory>
#include <cstdint>

#pragma warning(disable: 4503)

namespace async { namespace service {


	namespace details
	{
		// 内部使用
		template< typename T, typename PoolT >
		inline void release_buffer(T *p, PoolT &pool)
		{
			pool.deallocate(p, sizeof(T));
		}

		template< typename T, typename PoolT >
		inline T *create_buffer(PoolT &pool)
		{
			return reinterpret_cast<T *>(pool.allocate(sizeof(T)));
		}


		// MS BUG http://connect.microsoft.com/VisualStudio/feedback/details/772001
		/*template < typename T, typename...Args >
		inline T *object_allocate(Args&&... args)
		{
		return ::new(detail::create_buffer<T>()) T(std::forward<Args>(args)...);
		}*/

		// 内存申请释放
		template < typename T, typename PoolT >
		inline void object_deallocate(T *p, PoolT &pool)
		{
			p->~T();
			return details::release_buffer<T>(p, pool);
		}

		template < typename T, typename PoolT >
		inline T *object_allocate(PoolT &pool)
		{
			return ::new (details::create_buffer<T>(pool)) T;
		}


		template<typename T, typename PoolT, typename Arg1>
		inline T *object_allocate(PoolT &pool, Arg1 &&a1)
		{
			return ::new (details::create_buffer<T>(pool)) T(std::forward<Arg1>(a1));
		}

		template<typename T, typename PoolT, typename Arg1, typename Arg2>
		inline T *object_allocate(PoolT &pool, Arg1 &&a1, Arg2 && a2)
		{
			return ::new (details::create_buffer<T>(pool)) T(std::forward<Arg1>(a1), std::forward<Arg2>(a2));
		}

		template<typename T, typename PoolT, typename Arg1, typename Arg2, typename Arg3>
		inline T *object_allocate(PoolT &pool, Arg1&& a1, Arg2&& a2, Arg3&& a3)
		{
			return ::new (details::create_buffer<T>(pool)) T(std::forward<Arg1>(a1), std::forward<Arg2>(a2), std::forward<Arg3>(a3));
		}

		template<typename T, typename PoolT, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		inline T *object_allocate(PoolT &pool, Arg1&& a1, Arg2&& a2, Arg3&& a3, Arg4 &&a4)
		{
			return ::new (details::create_buffer<T>(pool)) T(std::forward<Arg1>(a1), std::forward<Arg2>(a2), std::forward<Arg3>(a3), std::forward<Arg4>(a4));
		}
	}


	template < typename T, typename PoolT, typename ...Args >
	std::shared_ptr<T> make_shared_object(PoolT &pool, Args &&...args)
	{
		return std::shared_ptr<T>(details::object_allocate<T>(pool, std::forward<Args>(args)...), 
			[&pool](T *p)
		{ 
			details::object_deallocate<T>(p, pool); 
		});
	}


}
}



#endif
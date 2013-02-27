#ifndef __ASYNC_IOCP_OBJECT_FACTORY_HPP
#define __ASYNC_IOCP_OBJECT_FACTORY_HPP

#include "../../memory_pool/sgi_memory_pool.hpp"
#include "../../memory_pool/fixed_memory_pool.hpp"

#include <memory>


namespace async
{

	namespace iocp
	{

		// 可以优化为线程相关内存池
		// 每个类型一个Memory Pool
		template< typename MemoryPoolT >
		struct object_pool_t
		{
			typedef MemoryPoolT	MemoryPool;
			static MemoryPool pool_;

			static MemoryPool &get_memory_pool()
			{
				return pool_;
			}
		};

		template < typename MemoryPoolT >
		typename object_pool_t<MemoryPoolT>::MemoryPool object_pool_t<MemoryPoolT>::pool_; 


		// 每个类型的内存的声请释放
		template< typename T >
		struct object_factory_t
		{
			typedef memory_pool::mt_memory_pool		PoolType;
			typedef object_pool_t<PoolType>			ObjectPoolType;
		};


		namespace detail
		{
			// 内部使用
			template< typename T >
			inline void release_buffer(T *p)
			{
				typedef object_pool_t<object_factory_t<T>::PoolType> pool_type;
				pool_type::get_memory_pool().deallocate(p, sizeof(T));
			}

			template< typename T >
			inline T *create_buffer()
			{
				typedef object_pool_t<object_factory_t<T>::PoolType> pool_type;
				return static_cast<T *>(pool_type::get_memory_pool().allocate(sizeof(T)));
			}
		}


		// 提供地址operator new/operator delete
		template < typename ImplT >
		struct new_delete_base_t
		{
			static void *operator new(size_t size)
			{
				assert(sizeof(ImplT) == size);

				typedef object_pool_t<object_factory_t<T>::PoolType> pool_type;
				return pool_type::get_memory_pool().allocate(size);
			}
			static void operator delete(void *ptr, size_t size)
			{
				assert(sizeof(ImplT) == size);

				if( ptr == NULL )
					return;

				typedef object_pool_t<object_factory_t<T>::PoolType> pool_type;
				return pool_type::get_memory_pool().deallocate(ptr, size);
			}
		};


		// 内存申请释放
		template < typename T >
		inline void object_deallocate(T *p)
		{
			p->~T();
			return detail::release_buffer<T>(p);
		}

		template < typename T >
		inline T *object_allocate()
		{
			return ::new (detail::create_buffer<T>()) T;
		}

		template < typename T, typename Arg1 >
		inline T *object_allocate(Arg1 &&a1)
		{
			return ::new (detail::create_buffer<T>()) T(a1);
		}

		template<typename T, typename Arg1, typename Arg2>
		inline T *object_allocate(Arg1 &&a1, Arg2 && a2)
		{
			return ::new (detail::create_buffer<T>()) T(a1, a2);
		}

		template<typename T, typename Arg1, typename Arg2, typename Arg3>
		inline T *object_allocate(Arg1&& a1, Arg2&& a2, Arg3&& a3)
		{
			return ::new (detail::create_buffer<T>()) T(a1, a2, a3);
		}

		template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
		inline T *object_allocate(Arg1&& a1, Arg2&& a2, Arg3&& a3, Arg4 &&a4)
		{
			return ::new (detail::create_buffer<T>()) T(a1, a2, a3, a4);
		}

		template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
		inline T *object_allocate(Arg1&& a1, Arg2&& a2, Arg3&& a3, Arg4 &&a4, Arg5 &&a5)
		{
			return ::new (detail::create_buffer<T>()) T(a1, a2, a3, a4, a5);
		}

		template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
		inline T *object_allocate(Arg1&& a1, Arg2&& a2, Arg3&& a3, Arg4 &&a4, Arg5 &&a5, Arg6 &&a6)
		{
			return ::new (detail::create_buffer<T>()) T(a1, a2, a3, a4, a5, a6);
		}

		template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
		inline T *object_allocate(Arg1&& a1, Arg2&& a2, Arg3&& a3, Arg4 &&a4, Arg5 &&a5, Arg6 &&a6, Arg7 &&a7)
		{
			return ::new (detail::create_buffer<T>()) T(a1, a2, a3, a4, a5, a6, a7);
		}

		template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
		inline T *object_allocate(Arg1&& a1, Arg2&& a2, Arg3&& a3, Arg4 &&a4, Arg5 &&a5, Arg6 &&a6, Arg7 &&a7, Arg8 &&a8)
		{
			return ::new (detail::create_buffer<T>()) T(a1, a2, a3, a4, a5, a6, a7, a8);
		}

		template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
		inline T *object_allocate(Arg1&& a1, Arg2&& a2, Arg3&& a3, Arg4 &&a4, Arg5 &&a5, Arg6 &&a6, Arg7 &&a7, Arg8 &&a8, Arg9 &&a9)
		{
			return ::new (detail::create_buffer<T>()) T(a1, a2, a3, a4, a5, a6, a7, a8, a9);
		}
	}

}



#endif
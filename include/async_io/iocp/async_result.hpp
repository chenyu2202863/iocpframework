#ifndef __IOCP_ASYNC_RESULT_HPP
#define __IOCP_ASYNC_RESULT_HPP

#include "../basic.hpp"
#include "pointer.hpp"
#include "object_factory.hpp"
#include "../../multi_thread/tls.hpp"

#include <functional>
#include <type_traits>


namespace async
{
	namespace iocp
	{
		enum error_code;

		inline u_long error_code_2_win32_error(error_code code)
		{
			return static_cast<u_long>(code);
		}

		inline error_code win32_error_2_error_code(u_long code)
		{
			return static_cast<error_code>(code);
		}

		// 回调接口
		typedef std::function<void(error_code , u_long)>	rw_callback_type;
		typedef std::function<void(error_code)>				accept_callback_type;
		typedef std::function<void(error_code)>				connect_callback_type;

		static std::tr1::_Ph<1> _Error;
		static std::tr1::_Ph<2> _Size;
		
		

		// Allocate IO Callback
	
		struct async_callback_base;


		template < typename T >
		struct io_async_callback_t;

		
		struct async_callback;
		typedef pointer_t< async_callback_base, io_async_callback_t<async_callback> > async_callback_base_ptr;

		

		//---------------------------------------------------------------------------
		// struct async_callback_base

		struct async_callback_base
			: public OVERLAPPED
		{
			async_callback_base();
			virtual ~async_callback_base();

			virtual void invoke(async_callback_base *p, error_code error, u_long size) = 0;

			template < typename KeyT, typename OverlappedT >
			static void call(KeyT *key, OverlappedT *overlapped, u_long size, u_long error)
			{
				async_callback_base *p(static_cast<async_callback *>(overlapped));
				
				p->invoke(p, win32_error_2_error_code(error), size);
			}
		};


		//---------------------------------------------------------------------------
		// class async_callback

		struct async_callback
			: public async_callback_base
		{
			rw_callback_type handler_;

			explicit async_callback(const rw_callback_type &callback)
				: handler_(std::move(callback))
			{}
			virtual ~async_callback();

		public:
			virtual void invoke(async_callback_base *p, error_code error, u_long size);

		private:
			async_callback();
		};

	

		// Memory Pool
		template< >
		struct object_factory_t<async_callback>
		{
			typedef memory_pool::fixed_memory_pool_t<true, sizeof(async_callback)>	PoolType;
			typedef object_factory_t<PoolType>										ObjectPoolType;
		};


		namespace detail
		{
			template < typename CallbackT >
			struct tls_memory_pool_t
			{
				typedef typename std::aligned_storage<
					sizeof(CallbackT),
					std::alignment_of<CallbackT>::value 
				>::type StorageBuffer;

				static __declspec(thread) StorageBuffer buf_;

				template < typename HandlerT >
				static CallbackT *object_allocate(const HandlerT &handler)
				{
					return ::new (&buf_) CallbackT(handler);
				}

				static void object_deallocate(CallbackT *p)
				{
					p->~CallbackT();
				}
			};

			template< typename CallbackT >
			__declspec(thread) typename tls_memory_pool_t<CallbackT>::StorageBuffer tls_memory_pool_t<CallbackT>::buf_;

			typedef tls_memory_pool_t<async_callback> tls_async_memory_pool;
		}


		// Pointer Realase

		template < >
		struct io_async_callback_t<async_callback>
		{
			static async_callback_base *allocate(const rw_callback_type &handler)
			{
				//return detail::TlsAsyncMemoryPool::ObjectAllocate(handler);
				return reinterpret_cast<async_callback_base *>(object_allocate<async_callback>(handler));
			}

			void operator()(async_callback_base *p)
			{
				//detail::TlsAsyncMemoryPool::ObjectDeallocate(reinterpret_cast<AsyncCallback *>(p));
				object_deallocate<async_callback>(static_cast<async_callback *>(p));
			}
		};


		template < typename T >
		inline async_callback_base *make_async_callback(const rw_callback_type &handler)
		{
			return io_async_callback_t<T>::allocate(handler);
		}
	}
}



#endif
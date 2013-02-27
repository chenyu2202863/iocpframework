#ifndef __FILE_SYSTEM_FILE_CHANGE_CALLBACK_HPP
#define __FILE_SYSTEM_FILE_CHANGE_CALLBACK_HPP

#include <functional>
#include <type_traits>

namespace async
{
	namespace iocp
	{

		// forward declare 
		typedef std::function<void(u_long, error_code, FILE_NOTIFY_INFORMATION *)> file_change_callback_type;

		struct file_change_callback;


		// IO Callback Allocator & Deallocator
		template < >
		struct io_async_callback_t< file_change_callback >
		{
			file_change_callback *operator()(const file_change_callback_type &handler)
			{
				return object_allocate<file_change_callback>(handler);
			}

			void operator()(file_change_callback *p)
			{
				object_deallocate<file_change_callback>(p);
			}	
		};

		typedef pointer_t<file_change_callback, io_async_callback_t<file_change_callback>> file_change_callback_ptr;
	
		template < typename HandlerT >
		inline file_change_callback *make_async_callback(const HandlerT &handler)
		{
			return io_async_callback_t<file_change_callback>()(handler);
		}

	
		// IO Callback
		struct file_change_callback
			: public async_callback_base
		{
			static const size_t BUFFER_LEN	= 64 * 1024;
			static const size_t PADDING		= sizeof(u_long);

			file_change_callback_type handler_;
			std::aligned_storage<BUFFER_LEN, PADDING>::type buffer_;

			file_change_callback(const file_change_callback_type &handler)
				: handler_(handler)
			{
			}

			virtual ~file_change_callback()
			{}

			virtual void invoke(async_callback_base *p, error_code error, u_long size)
			{
				FILE_NOTIFY_INFORMATION *notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(&buffer_);

				handler_(size, error, notifyInfo);

				file_change_callback_ptr ptr(reinterpret_cast<file_change_callback *>(p));
			}
		};


		// Memory Pool
		template < >
		struct object_factory_t< file_change_callback >
		{
			typedef memory_pool::fixed_memory_pool_t<true, sizeof(file_change_callback)>	PoolType;
			typedef object_pool_t<PoolType>													ObjectPoolType;
		};

	}
}





#endif
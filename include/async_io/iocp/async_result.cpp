#include "async_result.hpp"


namespace async
{
	namespace iocp
	{

		async_callback_base::async_callback_base()
		{
			RtlZeroMemory((OVERLAPPED *)this, sizeof(OVERLAPPED));
		}

		async_callback_base::~async_callback_base()
		{}

			


		async_callback::~async_callback()
		{

		}

		void async_callback::invoke(async_callback_base *p, error_code error, u_long size)
		{
			if( handler_ != 0 )
				handler_(error, size);

			async_callback_base_ptr ptr(p);
		}
	}
}
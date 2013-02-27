#ifndef __ASYNC_URDL_DETAIL_COROUTINE_HPP
#define __ASYNC_URDL_DETAIL_COROUTINE_HPP



namespace urdl 
{
	namespace detail 
	{

		class coroutine
		{
		protected:
			coroutine() 
				: coro_value_(0) 
			{}
			int coro_value_;
		};

#define URDL_CORO_BEGIN \
	switch (this->coroutine::coro_value_) \
		{ \
	case 0:

#define URDL_CORO_YIELD_IMPL(s,n) \
		do \
		{ \
			this->coroutine::coro_value_ = n; \
			s; \
			return; \
	case n: \
			; \
		} while (0);

# define URDL_CORO_YIELD(s) URDL_CORO_YIELD_IMPL(s, __COUNTER__ + 1)

#define URDL_CORO_END \
		}

	}
}


#endif 

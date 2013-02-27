#ifndef __ASYNC_TIMER_TIMER_HPP
#define __ASYNC_TIMER_TIMER_HPP

#include "../basic.hpp"
#include "impl/basic_timer.hpp"
#include "impl/timer_impl.hpp"


namespace async
{
	namespace timer
	{
		typedef impl::basic_timer_t<impl::waitable_timer, iocp::io_dispatcher>		timer_handle;

		typedef std::shared_ptr<timer_handle>										timer_handle_ptr;
	}
}



#endif
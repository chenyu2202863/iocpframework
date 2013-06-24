#ifndef __ASYNC_TIMER_TIMER_HPP
#define __ASYNC_TIMER_TIMER_HPP

#include "../basic.hpp"
#include "impl/basic_timer.hpp"
#include "impl/timer_impl.hpp"
#include "impl/timer_service.hpp"

namespace async { namespace timer {

	typedef timer_service_t<waitable_timer_t>	win_timer_service_t;
	typedef basic_timer_t<win_timer_service_t>	timer_handle;
	typedef std::shared_ptr<timer_handle>		timer_handle_ptr;

}
}



#endif
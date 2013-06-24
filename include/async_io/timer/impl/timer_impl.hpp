#ifndef __TIMER_WAITEABLE_TIMER_HPP
#define __TIMER_WAITEABLE_TIMER_HPP

#include "../../basic.hpp"
#include "../../service/exception.hpp"


namespace async { namespace timer {


	// --------------------------------------------------
	// class WaitableTimer

	class waitable_timer_t
	{
	private:
		HANDLE timer_;
		long period_;		// 间隔时间
		long due_;			// 第一次后延长时间

	public:
		waitable_timer_t(long period, long due, bool manualReset = false, const wchar_t *timerName = nullptr);
		~waitable_timer_t();

	public:
		HANDLE native_handle()
		{
			return timer_;
		}
		const HANDLE native_handle() const
		{
			return timer_;
		}

		operator HANDLE()
		{
			return timer_;
		}
		operator const HANDLE() const
		{
			return timer_;
		}

		bool operator==(const waitable_timer_t &timer) const
		{
			return timer.timer_ == timer_;
		}

	public:
		void set_timer(long period, long delay);

		void cancel();

		void sync_wait() const;

		void async_wait();
	};
}
}



#endif
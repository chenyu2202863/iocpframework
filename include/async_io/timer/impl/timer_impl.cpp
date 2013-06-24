#include "timer_impl.hpp"

#include <Windows.h>

namespace async { namespace timer {

	waitable_timer_t::waitable_timer_t(long period, long due, bool manualReset, const wchar_t *timerName)
		: timer_(NULL)
		, period_(period)
		, due_(due)
	{
		timer_ = ::CreateWaitableTimerW(NULL, manualReset ? TRUE : FALSE, timerName);
	}

	waitable_timer_t::~waitable_timer_t()
	{
		if( timer_ != NULL )
		{
			::CloseHandle(timer_);
			timer_ = NULL;
		}
	}


	void waitable_timer_t::set_timer(long period, long delay)
	{
		assert(timer_ != NULL);

		LARGE_INTEGER dueTime = {0};
		dueTime.QuadPart = -(delay * 10000000);

		period_ = period;
		due_ = delay;

		if( !::SetWaitableTimer(timer_, &dueTime, period, NULL, NULL, TRUE) )
			throw service::win32_exception_t("SetWaitableTimer");
	}

	void waitable_timer_t::cancel()
	{
		assert(timer_ != NULL);
		if( !::CancelWaitableTimer(timer_) )
			throw service::win32_exception_t("CancelWaitableTimer");
	}

	void waitable_timer_t::sync_wait() const
	{
		assert(timer_ != NULL);

		DWORD res = ::WaitForSingleObject(timer_, period_);
		if( res == WAIT_FAILED )
			throw service::win32_exception_t("WaitForSingleObject");
	}

	void waitable_timer_t::async_wait()
	{
		assert(timer_ != NULL);

		set_timer(period_, due_);
	}
}}
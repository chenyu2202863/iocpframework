#ifndef __TIMER_WAITEABLE_TIMER_HPP
#define __TIMER_WAITEABLE_TIMER_HPP

#include "../../basic.hpp"
#include "../../iocp/win_exception.hpp"


namespace async
{
	namespace timer
	{
		namespace impl
		{
			// --------------------------------------------------
			// class WaitableTimer

			class waitable_timer
			{
			private:
				HANDLE timer_;
				long period_;		// 间隔时间
				long due_;			// 第一次后延长时间
				
			public:
				waitable_timer(long period, long due, bool manualReset = false, const wchar_t *timerName = NULL)
					: timer_(NULL)
					, period_(period)
					, due_(due)
				{
					timer_ = ::CreateWaitableTimerW(NULL, manualReset ? TRUE : FALSE, timerName);
				}

				~waitable_timer()
				{
					if( timer_ != NULL )
					{
						::CloseHandle(timer_);
						timer_ = NULL;
					}
				}

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

				bool operator==(const waitable_timer &timer) const
				{
					return timer.timer_ == timer_;
				}

			public:
				void set_timer(long period, long delay)
				{
					assert(timer_ != NULL);

					LARGE_INTEGER dueTime = {0};
					dueTime.QuadPart = -(delay * 10000000);

					period_ = period;
					due_ = delay;

					if( !::SetWaitableTimer(timer_, &dueTime, period, NULL, NULL, TRUE) )
						throw iocp::win32_exception("SetWaitableTimer");
				}

				void cancel()
				{
					assert(timer_ != NULL);
					if( !::CancelWaitableTimer(timer_) )
						throw iocp::win32_exception("CancelWaitableTimer");
				}

				void sync_wait() const
				{
					assert(timer_ != NULL);

					DWORD res = ::WaitForSingleObject(timer_, period_);
					if( res == WAIT_FAILED )
						throw iocp::win32_exception("WaitForSingleObject");
				}

				void async_wait()
				{
					assert(timer_ != NULL);

					set_timer(period_, due_);
				}
			};

		}
	}
}



#endif
#ifndef __MULTI_THREAD_THREADPOOL_TASK_ADAPTOR_HPP
#define __MULTI_THREAD_THREADPOOL_TASK_ADAPTOR_HPP

#include <functional>


namespace multi_thread
{
	namespace threadpool
	{


		// ----------------------------------------
		// 不带参数，返回类型为void

		typedef std::tr1::function<void()> task_function;


		// -----------------------------------------
		// 带有优先级，可以进行<比较

		class prio_task_function
		{
			size_t priority_;
			task_function func_;

		public:
			typedef void ResultType;

		public:
			prio_task_function(size_t priority, const task_function &fun)
				: priority_(priority)
				, func_(fun)
			{}

		public:
			ResultType operator()() const
			{
				if( func_ )
					func_();
			}

			bool operator<(const prio_task_function &rhs) const
			{
				return priority_ < rhs.priority_;
			}
		};


		// ------------------------------------------------
		// 等待超时循环执行

		class looped_task_function
		{
		private:
			task_function func_;
			unsigned long millseconds_;
			HANDLE &exit_;

		public:
			typedef void ResultType;

		public:
			explicit looped_task_function(const task_function &taskFunc, HANDLE &exit, unsigned long millseconds = 0)
				: func_(taskFunc)
				, exit_(exit)
				, millseconds_(millseconds)
			{}

		public:
			ResultType operator()() const
			{
				while( 1 )
				{
					DWORD ret = ::WaitForSingleObject(exit_, millseconds_);
					if( ret == WAIT_OBJECT_0 )
						return;
					else if( ret == WAIT_TIMEOUT && func_ )
						func_();
				}
			}
		};


		// -----------------------------------------------
		// 等待事件或者超时执行

		class wait_handle_task_function
		{
			task_function func_;
			HANDLE &wait_;
			HANDLE &exit_;
			unsigned long millseconds_;

		public:
			typedef void ResultType;

		public:
			wait_handle_task_function(const task_function &task, HANDLE &wait, HANDLE &exit, unsigned long timeOut = INFINITE)
				: func_(task)
				, wait_(wait)
				, exit_(exit)
				, millseconds_(timeOut)
			{}

			ResultType operator()() const
			{
				HANDLE handle[] = { wait_, exit_ };

				while(1)
				{
					DWORD ret = ::WaitForMultipleObjects(_countof(handle), handle, FALSE, millseconds_);
					if( ret == WAIT_OBJECT_0 + 1 )
						return;
					else if( ret == WAIT_OBJECT_0 || ret == WAIT_TIMEOUT )
						func_();
					else
					{
						assert(0);
						return;
					}
				}
			}
	
		};
	}
}






#endif
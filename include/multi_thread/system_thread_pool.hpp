#ifndef __MULTI_THREAD_THREADPOOL_HPP
#define __MULTI_THREAD_THREADPOOL_HPP

#pragma warning(disable:4100)	// 消除未引用形参警告

/*
配置
	set_threadpool_max_num

分配内存hook函数
	handler_allocate_hook
	handler_deallocate_hook

系统线程池
	queue_work_item_pool
	queue_timer_pool
	io_completion_pool
	wait_object_pool

todo:
	VISTA以上系统线程池
*/


namespace multi_thread
{

	/*
	线程池的组件及其行为特性：

	Timer						RegisterWait					BindIO							QueueUser

	线程的初始数值		总是1						0								0								0

	当创建一个线程时		当调用第一个线程池函数时		每63个注册对象一个线程				系统会使用试探法，有如下因素影响：
	1. 自从添加线程后已经过去一定时间(单位ms)
	2. 使用WT_EXECUTELONGFUNCTION
	3. 已经排队的工作项目的数量超过了某个阀值

	当线程被撤销时		当进程终止时					当已经注册的等待对象数量为0		当线程没有未处理的IO请求
	并且已经空闲了一个阀值的周期		当线程空闲了一个阀值的周期

	线程如何等待			待命等待						WaitForMultipleObjects			待命等待							GetQueuedCompletionStatus

	什么唤醒线程			等待定时器通知排队的APC		内核对象变为已通知状态				排队的用户APC和已完成的IO请求		展示已完成的状态和IO请示




	*/




	/*
	WT_EXECUTEDEFAULT-- 默认情况下，该回调函数被排队到非 i/o 工作线程。是通过IO完成端口实现的
	回调函数被排队，这意味着它们不能执行一个 alertable 的等待 I/O 完成端口使用的线程。 
	因此，如果完成 I/O 但生成 APC可能无限期等待因为不能保证该线程在回调完成后将进入 alertable 等待状态。

	WT_EXECUTEINIOTHREAD--回调函数是一个 I/O 工作线程来排队。 此线程将执行一个 alertable 的等待。 
	这是效率较低，因此，仅当回调到当前线程生成 apc 和后该线程返回到线程池，则应执行 APC 时，才应使用此标志。 
	回调函数是作为 APC 排队。 如果该函数执行 alertable 等待操作，请务必解决可重入性问题。

	WT_EXECUTEINPERSISTENTTHREAD--回调函数被排队到永远不会终止的线程。 它不能保证在同一个线程在每次使用。
	此标志应仅用于短任务，或它可能会影响其他计时器操作。 
	请注意当前没有工作程序线程是真正持久，但如果有任何挂起 I/O 请求，不会终止工作线程。

	WT_EXECUTELONGFUNCTION--回调函数可以执行一个长时间的等待。 此标志可以帮助决定如果它应创建一个新线程的系统。

	WT_TRANSFER_IMPERSONATION--回调函数将使用在当前的访问令牌，它是一个过程还是模拟令牌。
	如果未指定此标志，则回调函数仅执行进程令牌。

	*/

	enum { MAX_THREADS = 10 };

	// 设置最大线程池数量
	inline void set_threadpool_max_num(DWORD dwFlag, DWORD dwThreads = MAX_THREADS)
	{
		WT_SET_MAX_THREADPOOL_THREADS(dwFlag, dwThreads);
	}

	// Handler Allocate / Deallocate HOOK

	inline void *handler_allocate_hook(size_t sz, ...)
	{
		return ::operator new(sz);
	}
	inline void handler_deallocate_hook(size_t sz, void *p, ...)
	{
		sz;
		::operator delete(p);
	}




	namespace detail
	{
		struct param_base
		{
			param_base()
			{}

			virtual ~param_base()
			{}

			void invoke()
			{
				callback(FALSE);
			}
			void invoke(BOOLEAN wait)
			{
				callback(wait);
			}

			virtual void callback(BOOLEAN wait) = 0;
		};

		typedef std::tr1::shared_ptr<param_base> param_base_ptr;

		template < typename HandlerT >
		struct thread_param_t
			: public param_base
		{
			HandlerT handler_;

			thread_param_t(const HandlerT &handler)
				: param_base()
				, handler_(handler)
			{}

			virtual void callback(BOOLEAN wait)
			{
				handler_(wait);
			}
		};


		// handler 分配器
		template<typename T>
		void _HandlerDeallocate(T *p)
		{
			p->~T();
			handler_deallocate_hook(sizeof(T), p, &p);
		}

		template<typename HandlerT>
		detail::param_base_ptr _HandlerAllocate(const HandlerT &handler)
		{
			typedef detail::thread_param_t<HandlerT> Thread;
			void *p = 0;
			p = handler_allocate_hook(sizeof(Thread), p);
			new (p) Thread(handler);

			detail::param_base_ptr threadParam(static_cast<detail::param_base *>(p), _HandlerDeallocate<detail::param_base>);
			return threadParam;
		}

	}



	//----------------------------------------------------------------------------------
	// class QueueWorkItem

	class queue_work_item_pool
	{
	private:
		detail::param_base_ptr callback_;

	public:
		queue_work_item_pool()
		{}
		template<typename HandlerT>
		explicit queue_work_item_pool(const HandlerT &handle)
			: callback_(detail::_HandlerAllocate(handle))
		{}

	public:
		template<typename HandlerT>
		bool call(const HandlerT &handler, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			if( callback_ == 0 )
				callback_ = detail::_HandlerAllocate(handler);

			assert(callback_);
			return call(nFlags);
		}

		bool call(ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			assert(callback_);
			return TRUE == ::QueueUserWorkItem(&queue_work_item_pool::proc_impl, callback_.get(), nFlags);
		}

	private:
		static DWORD WINAPI proc_impl(LPVOID pvParam)
		{
			detail::param_base *p(static_cast<detail::param_base *>(pvParam));

			try 
			{
				p->invoke();
			}
			catch(...) 
			{}

			return 0;
		}

	private:
		queue_work_item_pool(const queue_work_item_pool &);
		queue_work_item_pool &operator=(const queue_work_item_pool &);
	};



	//----------------------------------------------------------------------------------
	// class QueueTimer

	/*
	WT_EXECUTEDEFAULT				让非IO组件的线程来处理工作项目
	WT_EXECUTEINIOTHREAD			如果想要在某个时间发出一个异步IO请求
	WT_EXECUTEPERSISTENTTHREAD		如果想要让一个绝不会终止运行的线程来处理该工作项目
	WT_EXECUTELONGFUNCTION			如果认为工作项目需要很长时间来运行


	WT_EXECUTEDEFAULT
	0x00000000 默认情况下，回调函数被排队到非 i/o 工作线程。

	WT_EXECUTEINTIMERTHREAD
	0x00000020 由本身的计时器线程调用回调函数。 此标志应仅用于短任务，或它可能会影响其他计时器操作。 回调函数是作为 APC 排队。 它不应执行报警等操作。

	WT_EXECUTEINIOTHREAD
	0x00000001 回调函数被排队以一个 I/O 工作线程。 如果该函数应执行线程等待报警状态中时，才应使用此标志。 回调函数是作为 APC 排队。 如果该函数执行报警等操作，，请务必解决可重入性问题。

	WT_EXECUTEINPERSISTENTTHREAD
	0x00000080 回调函数被排队到永远不会终止的线程。 它并不能保证在同一线程在每次使用。 此标志应仅用于短任务，或它可能会影响其他计时器操作。 请注意目前没有工作程序线程是真正持久虽然没有工作程序线程将终止如果有任何挂起的 I/O 请求。

	WT_EXECUTELONGFUNCTION
	0x00000010 回调函数可以执行一个长时间的等待。 此标志可帮助系统，决定是否它应创建一个新的线程。

	WT_EXECUTEONLYONCE
	0x00000008 计时器将一次只能设置为已发送信号状态。 如果设置了此标志 期 参数必须为零。

	WT_TRANSFER_IMPERSONATION
	0x00000100 回调函数将使用当前的访问令牌，不论是进程或模拟令牌。 如果不指定此标志，则回调函数只执行进程令牌。

	*/
	class queue_timer_pool
	{
		detail::param_base_ptr callback_;
		HANDLE newTimer_;

	public:
		queue_timer_pool()
			: newTimer_(0)
		{}
		template<typename HandlerT>
		explicit queue_timer_pool(const HandlerT &handler)
			: callback_(detail::_HandlerAllocate(handler))
			, newTimer_(NULL)
		{}

		~queue_timer_pool()
		{
			if( newTimer_ != NULL )
				cancel();

			assert(callback_);
		}

	public:
		template < typename HandlerT >
		bool call(const HandlerT &handler, DWORD dwDueTime, DWORD dwPeriod, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			if( *callback_ == 0 )
				callback_ = detail::_HandlerAllocate(handler);

			assert(handler);
			return call(dwDueTime, dwPeriod, nFlags);
		}

		bool call(DWORD dwDueTime, DWORD dwPeriod, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			assert(callback_);
			return TRUE == ::CreateTimerQueueTimer(&newTimer_, NULL, 
				reinterpret_cast<WAITORTIMERCALLBACK>(&queue_timer_pool::proc_impl), 
				callback_.get(), dwDueTime, dwPeriod, nFlags);
		}

		// 改变定时器的到期时间和到期周期
		bool change(DWORD dwDueTime, DWORD dwPeriod)
		{
			assert(newTimer_ != NULL);
			assert(callback_);
			return TRUE == ::ChangeTimerQueueTimer(NULL, newTimer_, dwDueTime, dwPeriod);
		}

		// 不应该为hCompletionEvent传递INVALID_HANDLE_VALUE
		// 如果是传递的NULL，则该函数马上返回
		// 如果是传递的一个事件句柄，则会马上返回，并且当定时器所有已经排队的工作项目完成后，会触发此事件(不应该认为触发此事件)
		bool cancel(HANDLE hCompletionEvent = NULL)
		{
			assert(newTimer_ != NULL);
			assert(callback_);
			if( ::DeleteTimerQueueTimer(NULL, newTimer_, hCompletionEvent) )
			{
				newTimer_ = NULL;
				return true;
			}

			return false;
		}

	private:
		static void WINAPI proc_impl(PVOID pvParam, BOOL bTimeout)
		{
			detail::param_base *p = static_cast<detail::param_base *>(pvParam);

			try 
			{
				p->invoke(static_cast<BOOLEAN>(bTimeout));
			}
			catch(...) 
			{}
		}

	private:
		queue_timer_pool(const queue_timer_pool &);
		queue_timer_pool &operator=(const queue_timer_pool &);
	};



	/*
	投递一个异步IO操作,在IO完成端口上，回调函数也是由线程池线程来执行

	服务器应用程序发出某些异步IO请求，当这些请求完成时，需要让一个线程池准备好来处理已完成的IO请求。
	BindIoCompletionCallback在内部调用CreateIoCompletionPort，传递hDevice和内部完成端口的句柄。
	调用该函数可以保证至少有一个线程始终在非IO组件中，与设备相关的完成键是重叠完成例程的地址。
	这样，当该设备的IO运行完成时，非IO组件就知道调用哪个函数，以便能够处理已完成IO请求


	*/

	//----------------------------------------------------------------------------------
	// class IOCompletionPool 

	class io_completion_pool
	{
		detail::param_base_ptr callback_;

	public:
		template<typename HandlerT>
		io_completion_pool(const HandlerT &handler)
			: callback_(detail::_HandlerAllocate(handler))
		{}

	private:
		io_completion_pool(const io_completion_pool &);
		io_completion_pool &operator=(const io_completion_pool &);

	public:
		bool call(HANDLE hBindHandle)
		{
			return ::BindIoCompletionCallback(hBindHandle, 
				&io_completion_pool::proc_impl, 0) == TRUE;
		}

	private:
		static void WINAPI proc_impl(DWORD dwError, DWORD dwNum, OVERLAPPED *pOverlapped)
		{
			// .... to do
			//static_assert(0, "to do");
		}
	};



	/*
	WT_EXECUTEDEFAULT	0x00000000 
	默认情况下，该回调函数被排队到非 i/o 工作线程。

	WT_EXECUTEINIOTHREAD	0x00000001 
	回调函数是一个 I/O 工作线程来排队。 如果该函数应等待 alertable 状态的线程中执行，应使用此标志。 
	回调函数是作为 APC 排队。 如果该函数执行 alertable 等待操作，，请务必解决可重入性问题。

	WT_EXECUTEINPERSISTENTTHREAD 0x00000080 
	回调函数被排队到永远不会终止的线程。 它不能保证在同一个线程在每次使用。 此标志应仅用于短任务，或它可能会影响其他等待操作。 
	请注意当前没有工作程序线程是真正持久的虽然没有工作程序线程，如果有任何挂起 I/O 请求会终止。

	WT_EXECUTEINWAITTHREA 0x00000004 
	由本身的等待线程调用回调函数。 此标志应仅用于短任务，或它可能会影响其他等待操作。 
	如果另一个线程获取排他锁和调用回调函数时的 UnregisterWait 或 UnregisterWaitEx 函数正试图获取相同的锁，就会发生死锁。

	WT_EXECUTELONGFUNCTION	0x00000010 
	回调函数可以执行一个长时间的等待。 此标志可以帮助决定如果它应创建一个新线程的系统。

	WT_EXECUTEONLYONCE	0x00000008 
	回调函数调用一次该线程将不再等待该句柄。 否则，每次等待操作完成之前等待操作被取消，是重置计时器。

	WT_TRANSFER_IMPERSONATION	0x00000100 
	回调函数将使用在当前的访问令牌，它是一个过程还是模拟令牌。 如果未指定此标志，则回调函数仅执行进程令牌。

	*/

	//----------------------------------------------------------------------------------
	// class WaitObjectPool

	class wait_object_pool
	{
	private:
		detail::param_base_ptr callback_;
		HANDLE waitObject_;

	public:
		wait_object_pool()
			: waitObject_(0)
		{}
		template<typename HandlerT>
		explicit wait_object_pool(const HandlerT &handler)
			: callback_(detail::_HandlerAllocate(handler))
			, waitObject_(0)
		{}
		~wait_object_pool()
		{

		}

	private:
		wait_object_pool(const wait_object_pool &);
		wait_object_pool &operator=(const wait_object_pool &);


	public:
		template<typename HandlerT>
		bool call(const HandlerT &handler, HANDLE hObject, ULONG dwWait = INFINITE, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			if( *callback_ == 0 )
				callback_ = detail::_HandlerAllocate(handler);

			assert(*callback_);
			return call(hObject, dwWait, nFlags);
		}

		bool call(HANDLE hObject, ULONG dwWait = INFINITE, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			assert(callback_);
			return TRUE == ::RegisterWaitForSingleObject(&waitObject_, hObject, 
				&wait_object_pool::proc_impl, callback_.get(), dwWait, nFlags);
		}	
		// 取消
		bool cancel(HANDLE hCompletion = NULL)
		{
			assert(callback_);
			assert(waitObject_ != NULL);
			return TRUE == ::UnregisterWaitEx(waitObject_, hCompletion);
		}


	private:
		static void WINAPI proc_impl(PVOID pvParam, BOOLEAN bTimerOrWaitFired)
		{
			detail::param_base *p(static_cast<detail::param_base *>(pvParam));

			try 
			{
				p->invoke(bTimerOrWaitFired);
			}
			catch(...) 
			{}
		}
	};


}

#endif //
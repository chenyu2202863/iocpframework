#ifndef __MULTI_THREAD_AUTO_LOCK_HPP
#define __MULTI_THREAD_AUTO_LOCK_HPP

#include <Windows.h>
#include <cassert>
#include <limits>

#ifdef max
#undef max 
#endif

/*
RAII helper class
	auto_lock_t			
	auto_lock_ptr_t
	

Lock class
	lock null				无操作
	spin lock				旋转锁
	critical_section		临界区锁
	shared_rw_lock			共享读写锁(VSITA以上)
	exclusive_rw_lock		独占读写锁(VSITA以上)
	mutex					互斥锁
	event_t					事件锁
	condition				条件变量
		event_condition		事件模拟(XP)
		semaphore_condition 信号模拟(XP)
		ms_cs_condtion		(VISTA以上)
		ms_rw_condtion		(VISTA以上)

*/


namespace multi_thread
{
	//-------------------------------------------------------
	// auto_lock_t

	template < typename T >
	class auto_lock_t
	{
	private:
		T &m_lock;

	public:
		explicit auto_lock_t(T &lock)
			: m_lock(lock)
		{
			m_lock.lock();
		}
		~auto_lock_t()
		{
			m_lock.unlock();
		}

	private:
		auto_lock_t(const auto_lock_t &);
		auto_lock_t &operator=(const auto_lock_t &);
	};


	// ------------------------------------------------
	// class AutoLockPtr

	// wrapper for a volatile pointer

	template < typename T, typename MutexT >
	class auto_lock_ptr_t
	{
		T *obj_;
		MutexT &lock_;

	public:
		auto_lock_ptr_t(T &obj, MutexT &mutex)
			: obj_(&obj)
			, lock_(mutex)
		{
			lock_.lock();
		}
		~auto_lock_ptr_t()
		{
			lock_.unlock();
		}

		T &operator*() const
		{
			return *obj_;
		}

		T *operator->() const
		{
			return obj_;
		}
	};

	//-------------------------------------------------------
	// lock_null

	class lock_null
	{
	public:
		void lock(){}
		void unlock(){}
	};



	//-------------------------------------------------------
	// spin_lock

	class spin_lock
	{
	private:
		CRITICAL_SECTION mutex_;

	public:
		spin_lock(unsigned long dwCount = 4000) // <<Windows核心编程>> 建议值 
		{
			BOOL suc = ::InitializeCriticalSectionAndSpinCount(&mutex_, dwCount);	
			assert(suc);
		}

		~spin_lock() 
		{
			::DeleteCriticalSection(&mutex_);
		}

	public:
		void lock() 
		{
			::EnterCriticalSection(&mutex_);
		}

		void unlock() 
		{
			::LeaveCriticalSection(&mutex_);
		}

		operator CRITICAL_SECTION&()
		{
			return mutex_;
		}

		operator const CRITICAL_SECTION &() const
		{
			return mutex_;
		}
	};


	//-------------------------------------------------------
	// auto_critical_section

	class critical_section
	{
	private:
		CRITICAL_SECTION mutex_;

	public:
		critical_section() 
		{
			::InitializeCriticalSection(&mutex_);
		}

		~critical_section() 
		{
			::DeleteCriticalSection(&mutex_);
		}

	public:
		void lock() 
		{
			::EnterCriticalSection(&mutex_);
		}

		void unlock() 
		{
			::LeaveCriticalSection(&mutex_);
		}

		operator CRITICAL_SECTION&()
		{
			return mutex_;
		}

		operator const CRITICAL_SECTION &() const
		{
			return mutex_;
		}
	};


	//-------------------------------------------------------
	// class rd_lock_t 读写锁

	template < bool IsShared >
	class rd_lock_t
	{
		template < bool IsShared >
		struct select_lock_t;

		template <>
		struct select_lock_t<true>
		{
			rd_lock_t &rw_lock_;
			select_lock_t(rd_lock_t &lock)
				: rw_lock_(lock)
			{}

			void lock()
			{
				rw_lock_.r_lock();
			}
			void unlock()
			{
				rw_lock_.r_unlock();
			}
		};

		template <>
		struct select_lock_t<false>
		{
			rd_lock_t &rw_lock_;
			select_lock_t(rd_lock_t &lock)
				: rw_lock_(lock)
			{}

			void lock()
			{
				rw_lock_.w_lock();
			}

			void unlock()
			{
				rw_lock_.w_unlock();
			}
		};

	private:
		SRWLOCK rw_lock_;

	public:
		rd_lock_t()
		{
			::InitializeSRWLock(&rw_lock_);
		}
		~rd_lock_t()
		{
			// nothing
		}

	public:
		// 共享模式
		void r_lock()
		{
			::AcquireSRWLockShared(&rw_lock_);
		}
		// 独占模式
		void w_lock()
		{
			::AcquireSRWLockExclusive(&rw_lock_);
		}

		void r_unlock()
		{
			::ReleaseSRWLockShared(&rw_lock_);
		}
		void w_unlock()
		{
			::ReleaseSRWLockShared(&rw_lock_);
		}

	public:
		void lock()
		{
			select_lock_t<IsShared>(*this).lock();
		}

		void unlock()
		{
			select_lock_t<IsShared>(*this).unlock();
		}
	};

	typedef rd_lock_t<true>		shared_rw_lock;		// 共享读写锁
	typedef rd_lock_t<false>	exclusive_rw_Lock;	// 独占读写锁


	//-------------------------------------------------------
	// mutex

	class mutex
	{
	private:
		HANDLE mutex_;

	public:
		mutex(LPSECURITY_ATTRIBUTES lpMutexAttributes = 0, BOOL bInitialOwner = FALSE, LPCTSTR lpName = 0)
			: mutex_(0)
		{
			mutex_ = ::CreateMutex(lpMutexAttributes, bInitialOwner, lpName);
		}
		~mutex()
		{
			if( mutex_ != NULL )
			{
				BOOL suc = ::CloseHandle(mutex_);
				assert(suc);
			}
		}

	public:
		void lock()
		{
			assert(mutex_ != 0);
			::WaitForSingleObject(mutex_, INFINITE);
		}

		void unlock()
		{
			assert(mutex_ != 0);
			::ReleaseMutex(mutex_);
		}

		operator HANDLE &()
		{
			return mutex_;
		}
		operator const HANDLE &() const
		{
			return mutex_;
		}
	};

	//-------------------------------------------------------
	// event_t

	class event_t
	{
	public:
		HANDLE event_;

		event_t(HANDLE hEvent = 0) 
			: event_(hEvent)
		{ 
		}

		~event_t()
		{
			close();
		}

		bool create(LPCTSTR pstrName = 0, BOOL bManualReset = FALSE, BOOL bInitialState = FALSE, LPSECURITY_ATTRIBUTES pEventAttributes = 0)
		{
			assert(pstrName == 0 || !::IsBadStringPtr(pstrName,(UINT)-1));
			assert(event_==0);

			event_ = ::CreateEvent(pEventAttributes, bManualReset, bInitialState, pstrName);
			assert(event_!=0);

			return event_ != 0;
		}

		bool open(LPCTSTR pstrName, DWORD dwDesiredAccess = EVENT_ALL_ACCESS, BOOL bInheritHandle = TRUE)
		{
			assert(!::IsBadStringPtr(pstrName,(UINT)-1));
			assert(event_==0);

			event_ = ::OpenEvent(dwDesiredAccess, bInheritHandle, pstrName);

			return event_ != NULL;
		}

		bool is_open() const
		{
			return event_ != 0;
		}

		void close()
		{
			if( event_ == 0 ) 
				return;

			BOOL suc = ::CloseHandle(event_);
			assert(suc);
			event_ = 0;
		}

		void attach(HANDLE hEvent)
		{
			assert(event_==0);
			event_= hEvent;
		}  

		HANDLE detach()
		{
			HANDLE hEvent = event_;
			event_ = 0;
			return hEvent;
		}

		bool reset_event()
		{
			assert(event_!=0);
			return ::ResetEvent(event_) != 0;
		}

		bool set_event()
		{
			assert(event_!=0);
			return ::SetEvent(event_) != 0;
		}

		bool pulse_event()
		{
			assert(event_!=0);
			return ::PulseEvent(event_) != 0;
		}

		bool is_signalled() const
		{
			assert(event_!=0);
			return ::WaitForSingleObject(event_, 0) == WAIT_OBJECT_0;
		}

		bool wait_for_event(DWORD dwTimeout = INFINITE)
		{
			assert(event_!=0);
			return ::WaitForSingleObject(event_, dwTimeout) == WAIT_OBJECT_0;
		}

		operator HANDLE&()
		{ 
			return event_; 
		}

		operator const HANDLE &() const 
		{ 
			return event_; 
		}
	};



	// 条件变量

	namespace detail
	{
		// a semaphore object usually can be considered as an event set.

		class semaphore
		{
		private:
			HANDLE semaphore_;
			// 需要指定状态个数
			volatile long signal_count_;

		public:
			semaphore(long initCount = 0, long maxCount = std::numeric_limits<long>::max())
			{
				semaphore_		= ::CreateSemaphore( NULL, initCount, maxCount, NULL );
				::InterlockedExchange(&signal_count_, initCount);
			}

			~semaphore()
			{
				::CloseHandle(semaphore_);
			}

			// 设置状态为激活
			void signal(long count)
			{
				if( count <= 0 ) 
					return ;

				::ReleaseSemaphore(semaphore_, count, 0);
				::InterlockedExchangeAdd(&signal_count_, count);
			}

			// 等待状态为激活
			// 可以通过Signal来激活状态
			template < typename MutexT >
			bool wait(MutexT &mutex, DWORD waitTime)
			{
				mutex.unlock();

				DWORD ret = ::WaitForSingleObject(semaphore_, waitTime);
				::InterlockedDecrement(&signal_count_);

				mutex.lock();

				return ret == WAIT_OBJECT_0;
			}

			// 重置状态为非激活态
			void reset()
			{
				for(long i = 0; i < signal_count_; ++i)
				{
					::WaitForSingleObject( semaphore_, INFINITE );
				}

				::InterlockedExchange(&signal_count_, 0);
			}


		};

		// ------------------------------------------------------------
		// class MultipleEvent

		template< long _Size = 10 >
		class multiple_event_t
		{
			typedef critical_section	mutex;
			typedef auto_lock_t<mutex>		auto_lock;

		private:
			// event array
			HANDLE events_[_Size];
			// state pointer
			long pos_;
			// Mutex
			mutex mutex_;

		public:
			multiple_event_t()
			{
				for(long i = 0; i != _Size; ++i)
				{
					events_[i] = ::CreateEvent(NULL, TRUE, FALSE, NULL);
				}
				pos_ = -1;
			}

			// destructor
			~multiple_event_t()
			{
				for(long i = 0; i != _Size; ++ i)
				{
					::CloseHandle(events_[i]);
				}
			}

			// 设置多少个event为激活状态
			void signal(long count)
			{
				if( count <= 0 )
					return;

				auto_lock lock(mutex_);
				long max = pos_ + count >= _Size ? _Size - 1 : pos_ + count;
				for(long i = pos_ + 1; i <= max; ++ i)
				{
					::SetEvent(events_[i]);
				}

				pos_ += (max - pos_);
			}

			// 等待一个event
			template < typename MutexT >
			bool wait(MutexT &mutex, DWORD waitTime)
			{
				mutex.unlock();

				DWORD ret = 0;
				{
					auto_lock lock(mutex_);
					long waitPos = pos_ < 0 ? 0 : pos_;
					ret = ::SignalObjectAndWait(events_[waitPos], events_[waitPos], waitTime, FALSE);
					pos_ = waitPos == 0 ? pos_ : pos_ - 1;
				}


				mutex.lock();
				return ret == WAIT_FAILED;
			}

			// 重置所有事件为非激活状态
			void reset()
			{
				for(long i = 0; i <= pos_; ++i)
				{
					::ResetEvent(events_[i]);
				}

				pos_ = -1;
			}
		};

		class ms_condition
		{
			CONDITION_VARIABLE condition_;

		public:
			ms_condition()
			{
				::InitializeConditionVariable(&condition_);
			}

			void signal(long count)
			{
				if( count == 1 )
					::WakeConditionVariable(&condition_);
				else
					::WakeAllConditionVariable(&condition_);
			}

			template < typename MutexT >
			bool wait(MutexT &mutex, DWORD waitTime)
			{
				return _Wait(mutex, waitTime);
			}

			void reset()
			{
				// do nothing
			}

		private:
			bool _Wait(CRITICAL_SECTION &mutex, DWORD waitTime)
			{
				return ::SleepConditionVariableCS(&condition_, &mutex, waitTime) == TRUE;
			}

			bool _Wait(SRWLOCK &mutex, DWORD waitTime)
			{
				return ::SleepConditionVariableSRW(&condition_, &mutex, waitTime, CONDITION_VARIABLE_LOCKMODE_SHARED) == TRUE;
			}
		};
	}



	// -----------------------------------------------------
	// class Condition

	template< typename MultipleEventT >
	class condition_t
	{
	public:
		typedef MultipleEventT				MultipleEventType;

	private:
		// 事件
		MultipleEventType events_;
		// 需要等待的个数
		volatile long waitCount_;

	public:
		condition_t() 
			: waitCount_(0)
		{}

		// set the Condition state to the signaled state
		void signal()
		{
			if( ::InterlockedCompareExchange(&waitCount_, 0, 0) != 0 )
			{
				events_.signal( 1 );
				::InterlockedDecrement(&waitCount_);
			}
		}

		// wait the Condition until it 's signaled state
		template < typename MutexT >
		bool wait(MutexT &mutex, DWORD waitTime = INFINITE)
		{
			::InterlockedIncrement(&waitCount_);

			return events_.wait(mutex, waitTime);
		}

		// 设置所有event为有信号状态
		void broadcast()
		{

			if( ::InterlockedCompareExchange(&waitCount_, 0, 0) != 0 )
			{
				events_.signal(waitCount_);
				::InterlockedExchange(&waitCount_, 0);
			}
		}

		// 重置所有event为非激活状态
		void reset()
		{
			events_.reset();
		}
	};

	typedef condition_t<detail::multiple_event_t<>>	event_condition;
	typedef condition_t<detail::semaphore>			semaphore_condition;
	typedef condition_t<detail::ms_condition>		ms_cs_condtion;
	typedef condition_t<detail::ms_condition>		ms_rw_condtion;

}


#endif
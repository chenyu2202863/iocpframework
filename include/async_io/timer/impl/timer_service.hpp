#ifndef __TIMER_TIMER_SERVICE_HPP
#define __TIMER_TIMER_SERVICE_HPP

#include <unordered_map>
#include <mutex>
#include <thread>
#include <map>

#include "../../service/dispatcher.hpp"
#include "../../../exception/exception_base.hpp"
#include "../../../extend_stl/allocator/container_allocator.hpp"
#include "../../../memory_pool/sgi_memory_pool.hpp"

namespace async {

	class event_t
	{
	public:
		HANDLE event_;

		event_t(HANDLE hEvent = 0) 
			: event_(hEvent)
		{ 
			create();
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

	namespace timer {


		// alterable IO
		inline void WINAPI APCFunc(ULONG_PTR pParam)
		{
			// do nothing
		}


		// ------------------------------------------------
		// class TimerService
		template < typename TimerImplT >
		class timer_service_t
		{
		public:
			typedef TimerImplT								timer_impl_t;
			typedef std::shared_ptr<timer_impl_t>			timer_ptr;
			typedef service::io_dispatcher_t				service_type;

		private:
			struct callback_handler_t
			{
				std::function<void()> handler_;

				callback_handler_t(std::function<void()> && handler)
					: handler_(std::move(handler))
				{}
				~callback_handler_t()
				{}

				callback_handler_t(callback_handler_t && rhs)
					: handler_(std::move(rhs.handler_))
				{}

				void operator()(const std::error_code &, std::uint32_t)
				{
					handler_();
				}

				callback_handler_t(const callback_handler_t &rhs)
					: handler_(rhs.handler_)
				{}

			private:
				callback_handler_t &operator=(const callback_handler_t &);
			};
			typedef std::map<std::uint32_t, std::pair<timer_ptr, callback_handler_t>> timers_type;
		
			typedef std::mutex					Mutex;
			typedef std::unique_lock<Mutex>		Lock;

		private:
			timers_type timers_;								// Timers
			Mutex mutex_;

			service_type &io_;									// Asynchronous Callback service

			async::event_t update_;	
			std::thread thread_;								// WaitForMutipleObjectEx

		public:
			timer_service_t(service_type &io)
				: io_(io)
				, thread_(std::bind(&timer_service_t::_ThreadCallback, this))
			{}

			~timer_service_t()
			{
				stop();
				thread_.join();
			}

		private:
			timer_service_t(const timer_service_t &);
			timer_service_t &operator=(const timer_service_t &);


		public:
			void stop()
			{
				// 可提醒IO，退出监听线程
				::QueueUserAPC(APCFunc, thread_.native_handle(), NULL);
			}

			// 增加一个Timer
			template < typename HandlerT >
			std::uint32_t add_timer(long period, long due, HandlerT &&handler)
			{
				timer_ptr timer(new timer_impl_t(period, due));

				std::uint32_t id = (std::uint32_t)timer->native_handle();
				{
					Lock lock(mutex_);
					assert(timers_.size() <= MAXIMUM_WAIT_OBJECTS);
					if( timers_.size() > MAXIMUM_WAIT_OBJECTS )
						throw std::out_of_range("size must less than MAXIMUM_WAIT_OBJECTS");
					
					timers_.insert(std::move(std::make_pair(id, 
						std::make_pair(std::move(timer), std::move(callback_handler_t(std::forward<HandlerT>(handler)))))));
				}

				// 设置更新事件信号
				update_.set_event();

				return id;
			}

			template < typename HandlerT >
			void set_timer(std::uint32_t id, long period, long delay)
			{
				Lock lock(mutex_);
				auto iter = timers_.find(id);

				if( iter != timers_.end() )
				{
					iter->second.set_timer(period, delay);
				}
			}

			void erase_timer(std::uint32_t id)
			{
				{
					Lock lock(mutex_);
					auto iter = timers_.find(id);
					if(iter != timers_.end())
					{
						iter->second.first->cancel();
						timers_.erase(iter);
					}
				}

				// 设置更新事件信号
				update_.set_event();
			}

			void async_wait(std::uint32_t id)
			{
				Lock lock(mutex_);
				auto iter = timers_.find(id);
				if( iter != timers_.end() )
					iter->second.first->async_wait();
			}

		private:
			void _ThreadCallback()
			{
				std::vector<HANDLE> handles;

				typedef memory_pool::sgi_memory_pool_t<true, 256> pool_t;
				pool_t pool;
				typedef stdex::allocator::pool_allocator_t<char, pool_t> pool_allocator_t;
				pool_allocator_t pool_allocator(pool);

				handles.push_back(update_);

				while(true)
				{
					DWORD res = ::WaitForMultipleObjectsEx(handles.size(), &handles[0], FALSE, INFINITE, TRUE);
					if( res == WAIT_IO_COMPLETION )
						break;
					else if( res == WAIT_FAILED )
					{
						_Copy(handles);
						continue;
					}
					else if( res == WAIT_OBJECT_0 )
					{
						_Copy(handles);
						continue;
					}
					else if( res + WAIT_OBJECT_0 > timers_.size() )
					{
						continue;
					}
					

					Lock lock(mutex_);
					auto iter = timers_.find((std::uint32_t)handles[res]);
					if( iter != timers_.end() )
					{
						auto callback = iter->second.second;
						lock.unlock();
						io_.post(std::move(callback), pool_allocator);
					}
				}

				::OutputDebugStringW(L"Exit Timer Service Thread\n");
			}

			void _Copy(std::vector<HANDLE> &handles)
			{
				handles.clear();
				handles.push_back(update_);

				Lock lock(mutex_);
				for(auto iter = timers_.begin(); iter != timers_.end(); ++iter)
				{
					handles.push_back((HANDLE)iter->first);
				}
			}
		};
	}
}


#endif
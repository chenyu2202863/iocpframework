#ifndef __TIMER_TIMER_SERVICE_HPP
#define __TIMER_TIMER_SERVICE_HPP

#include <unordered_map>

#include "../../iocp/dispatcher.hpp"
#include "../../../multi_thread/thread.hpp"
#include "../../../exception/exception_base.hpp"



namespace async
{
	namespace timer
	{
		namespace impl
		{
			
			// alterable IO
			inline void WINAPI APCFunc(ULONG_PTR pParam)
			{
				// do nothing
			}

			namespace detail
			{
				template < typename T >
				struct find_timer_t
				{
					const T &timer_;

					find_timer_t(const T &timer)
						: timer_(timer)
					{}

					template < typename CallbackT >
					bool operator()(const std::pair<T, CallbackT> &val) const
					{
						return val.first == timer_;
					}
				};


				struct timer_callback
				{
					typedef std::function<void()> callback_type;
					callback_type handler_;

					timer_callback()
					{}

					timer_callback(const callback_type &handler)
						: handler_(std::move(handler))
					{}

					timer_callback(timer_callback &&rhs)
						: handler_(std::move(rhs.handler_))
					{}

					timer_callback &operator=(timer_callback &&rhs)
					{
						if( this != &rhs )
						{
							handler_ = std::move(rhs.handler_);
						}

						return *this;
					}

					void operator()(u_long, u_long)
					{
						handler_();
					}

					operator bool()
					{
						return handler_ != 0;
					}
				};
			}
			// ------------------------------------------------
			// class TimerService

			template < typename TimerT, typename ServiceT >
			class timer_service_t
			{
			public:
				typedef TimerT							timer_type;
				typedef std::shared_ptr<timer_type>		timer_ptr;
				typedef ServiceT						service_type;

			private:
				typedef detail::timer_callback						timer_callback_type;
				typedef std::map<timer_ptr, timer_callback_type>	timers_type;
				typedef typename timers_type::iterator				timers_iterator;
				typedef multi_thread::critical_section				Mutex;
				typedef multi_thread::auto_lock_t<Mutex>			Lock;

			private:
				timers_type timers_;									// Timers

				multi_thread::thread_impl_ex thread_;			// WaitForMutipleObjectEx
				multi_thread::event_t   update_;	
				Mutex mutex_;

				service_type &io_;								// Asynchronous Callback service

			private:
				timer_service_t(service_type &io)
					: io_(io)
				{
					update_.create();

					// 启动等待Timer线程
					thread_.register_callback(std::bind(&timer_service_t::_ThreadCallback, this));
					thread_.start();
				}

				~timer_service_t()
				{
					stop();
				}

				timer_service_t(const timer_service_t &);
				timer_service_t &operator=(const timer_service_t &);
				

			public:
				// 单件
				static timer_service_t<timer_type, service_type> &instance(service_type &io)
				{
					static timer_service_t<timer_type, service_type> service(io);
					return service;
				}

			public:
				void stop()
				{
					// 可提醒IO，退出监听线程
					::QueueUserAPC(APCFunc, thread_, NULL);

					thread_.stop();
				}

				// 增加一个Timer
				template < typename HandlerT >
				timer_ptr add_timer(long period, long due, const HandlerT &handler)
				{
					timer_ptr timer(new timer_type(period, due));
					
					{
						Lock lock(mutex_);
						timers_.insert(std::make_pair(timer, timer_callback_type(handler)));
					}
 
					// 设置更新事件信号
					update_.set_event();

					return timer;
				}
				timer_ptr add_timer(long period, long due)
				{
					return add_timer(period, due, timer_callback_type());
				}

				template < typename HandlerT >
				void set_timer(const timer_ptr &timer, const HandlerT &handler)
				{
					Lock lock(mutex_);
					timers_iterator iter = timers_.find(timer);

					if( iter == timers_.end() )
						return;

					iter->second = detail::timer_callback(handler);
				}

				void erase_timer(const timer_ptr &timer)
				{
					{
						Lock lock(mutex_);
						timers_iterator iter = timers_.find(timer);

						if( iter != timers_.end() )
							timers_.erase(iter);
					}
					

					// 设置更新事件信号
					update_.set_event();
				}

			private:
				DWORD _ThreadCallback()
				{
					std::vector<HANDLE> handles;

					while(!thread_.is_aborted())
					{
						// 如果有变化，则重置
						if( WAIT_OBJECT_0 == ::WaitForSingleObject(update_, 0) )
						{
							_Copy(handles);
						}
						
						// 防止刚启动时没有timer生成
						if( handles.size() == 0 )
						{
							if( WAIT_IO_COMPLETION == ::WaitForSingleObjectEx(update_, INFINITE, TRUE) )
								break;
							else
							{
								_Copy(handles);
							}
						}

						// 等待Timer到点
						if( handles.empty() )
							continue;

						DWORD res = ::WaitForMultipleObjectsEx(handles.size(), &handles[0], FALSE, INFINITE, TRUE);
						if( res == WAIT_IO_COMPLETION )
							break;

						if( WAIT_OBJECT_0 == ::WaitForSingleObject(update_, 0) )
						{
							update_.set_event();
							continue;
						}
						
						if( res == WAIT_FAILED )
						{
							update_.set_event();
							continue;
						}
						else if( res + WAIT_OBJECT_0 > timers_.size() )
							throw ::exception::exception_base("handle out of range");

						Lock lock(mutex_);
						if( !timers_.empty() )
						{
							timers_iterator iter = timers_.begin();
							std::advance(iter, WAIT_OBJECT_0 + res);

							if( iter->second )
								io_.post(iter->second);
						}
					}

					::OutputDebugStringW(L"Exit Timer Service Thread\n");
					return 0;
				}

				void _Copy(std::vector<HANDLE> &handles)
				{
					Lock lock(mutex_);
					handles.clear();

					for(timers_iterator iter = timers_.begin(); iter != timers_.end(); ++iter)
					{
						handles.push_back(*(iter->first));
					}
				}
			};

			
		}
	}
}


#endif
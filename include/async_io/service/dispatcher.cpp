#include "dispatcher.hpp"

#include <vector>
#include <thread>
#include <type_traits>

#include "iocp.hpp"
#include "exception.hpp"


namespace async { namespace service {

	size_t get_fit_thread_num(size_t perCPU)
	{
		return perCPU * std::thread::hardware_concurrency();
	}

	namespace 
	{
		// for exit
		void __stdcall ApcQueue(ULONG_PTR)
		{}
	}
	


	struct io_dispatcher_t::impl
	{
		// iocp Handle
		iocp_handle iocp_;

		// 线程容器
		std::vector<std::thread>	threads_;

		// 线程创建后初始化操作
		init_handler_t init_handler_;
		// 线程退出时结束操作
		uninit_handler_t uninit_handler_;

		// 错误消息回调
		error_msg_handler_t error_handler_;


		impl(size_t numThreads, const error_msg_handler_t &error_handler, const init_handler_t &init, const uninit_handler_t &unint)
			: error_handler_(error_handler)
			, uninit_handler_(unint)
			, init_handler_(init)
		{
			if( !iocp_.create(numThreads) )
				throw win32_exception_t("iocp_.Create()");

			// 创建指定的线程数
			threads_.reserve(numThreads);

			for(std::uint32_t i = 0; i != numThreads; ++i)
			{
				threads_.push_back(std::thread([this]
				{
					_thread_io();
				}));
			}
		}


		~impl()
		{
			try
			{		
				stop();
				iocp_.close();
			}
			catch(...)
			{
				assert(0 && __FUNCTION__);
				error_handler_("Unknown error!");
			}
		}

		void bind(HANDLE hHandle)
		{
			if( !iocp_.associate_device(hHandle, 0) )
				throw win32_exception_t("iocp_.associate_device");
		}


		void stop()
		{
			// 先停止所有的线程
			std::for_each(threads_.begin(), threads_.end(), 
				[this](std::thread &t)
			{ 
				::QueueUserAPC(ApcQueue, t.native_handle(), 0);
			});

			std::for_each(threads_.begin(), threads_.end(), [](std::thread &t)
			{
				t.join();
			});

			threads_.clear();
		}

		bool post_impl(const async_callback_base_ptr &val)
		{
			if( !iocp_.post_status(0, 0, static_cast<OVERLAPPED *>(val.get())) )
				throw win32_exception_t("iocp_.PostStatus");

			return true;
		}

		void _thread_io()
		{
			if( init_handler_ != nullptr )
				init_handler_();

			OVERLAPPED_ENTRY entrys[64] = {0};
			DWORD ret_number = 0;
			while(true)
			{
				::SetLastError(0);
				bool suc = iocp_.get_status_ex(entrys, ret_number);
				auto err = ::GetLastError();

				if( err == WAIT_IO_COMPLETION )
					break;

				try
				{
					for(auto i = 0; i != ret_number; ++i)
					{
						call(entrys[i].lpOverlapped, 
							entrys[i].dwNumberOfBytesTransferred,
							std::make_error_code((std::errc::errc)entrys[i].Internal));
					}
				}
				catch(const exception::exception_base &e)
				{
					e.dump();
					error_handler_(e.what());
					assert(0);
				}
				catch(const std::exception &e)
				{
					error_handler_(e.what());
					assert(0);
					// Opps!!
				}
				catch(...)
				{
					error_handler_("io_dispatcher fatal error: unknown msg");
					assert(0);
					// Opps!!
				}
			}

			if( uninit_handler_ != nullptr )
				uninit_handler_();

			::OutputDebugStringW(L"OVERLAPPED Thread Exit\n");
		}
	};

	io_dispatcher_t::io_dispatcher_t(const error_msg_handler_t &msg_handler, size_t numThreads/* = 0*/, const init_handler_t &init, const uninit_handler_t &unint)
		: impl_(new impl(numThreads, msg_handler, init, unint))
	{}

	io_dispatcher_t::~io_dispatcher_t()
	{
		stop();
	}


	void io_dispatcher_t::bind(void *hHandle)
	{
		impl_->bind(hHandle);
	}


	void io_dispatcher_t::stop()
	{
		impl_->stop();
	}

	bool io_dispatcher_t::_post_impl(const async_callback_base_ptr &val)
	{
		return impl_->post_impl(val);
	}

}

}
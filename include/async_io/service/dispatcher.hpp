#ifndef __IOCP_DISPATCHER_HPP
#define __IOCP_DISPATCHER_HPP

#include <cstdint>
#include <functional>

#include "async_result.hpp"


namespace async { namespace service {
		

		// 获取适合系统的线程数
		std::uint32_t get_fit_thread_num(size_t perCPU = 1);



		//------------------------------------------------------------------
		// class io_dispatcher

		class io_dispatcher_t
		{
		public:
			typedef std::function<void()>			init_handler_t;
			typedef std::function<void()>			uninit_handler_t;
			typedef std::function<void(const std::string &)> error_msg_handler_t;

		private:
			struct impl;
			std::unique_ptr<impl> impl_;

		public:
			explicit io_dispatcher_t(const error_msg_handler_t &msg_handler, size_t numThreads = get_fit_thread_num(), const init_handler_t &init = nullptr, const uninit_handler_t &unint = nullptr);
			~io_dispatcher_t();

		private:
			io_dispatcher_t(const io_dispatcher_t &);
			io_dispatcher_t &operator=(const io_dispatcher_t &);

		public:
			// 绑定设备到完成端口
			void bind(void *);
			// 向完成端口投递请求
			template<typename HandlerT>
			void post(HandlerT &&handler);
			// 停止服务
			void stop();

		private:
			bool _post_impl(const async_callback_base_ptr &);
		};


		template < typename HandlerT >
		void io_dispatcher_t::post(HandlerT &&handler)
		{
			async_callback_base_ptr async(make_async_callback(std::forward<HandlerT>(handler)));

			if( _post_impl(async) )
				async.release();
		}

	}
}



#endif
#ifndef __NETWORK_BASIC_STREAM_SOCKET_HPP
#define __NETWORK_BASIC_STREAM_SOCKET_HPP


namespace async
{
	namespace network
	{
		// -------------------------------------------------
		// class basic_stream_socket_t

		template < typename ProtocolT >
		class basic_stream_socket_t
		{
		public:
			typedef ProtocolT						protocol_type;
			typedef socket_handle_ptr::element_type::native_handle_type	native_handle_type;
			typedef socket_handle::dispatcher_type	dispatcher_type;	

		private:
			socket_handle_ptr impl_;

		public:
			explicit basic_stream_socket_t(dispatcher_type &io)
				: impl_(make_socket(io))
			{}
			explicit basic_stream_socket_t(const socket_handle_ptr &impl)
				: impl_(impl)
			{}

			basic_stream_socket_t(dispatcher_type &io, const protocol_type &protocol)
				: impl_(make_socket(io, protocol.family(), protocol.type(), protocol.protocol()))
			{}

		public:
			// 显示获取
			native_handle_type &native_handle() 
			{
				return impl_->native_handle();
			}
			const native_handle_type &native_handle() const
			{
				return impl_->native_handle();
			}


		public:
			void open(const protocol_type &protocol = protocol_type::v4())
			{
				impl_->open(protocol.family(), protocol.type(), protocol.protocol());
			}
	
			bool is_open() const
			{
				return impl_->is_open();
			}

			void shutdown(int shut)
			{
				impl_->shutdown(shut);
			}
			void close()
			{
				impl_->shutdown(SD_BOTH);
				impl_->close();
			}

			void cancel()
			{
				return impl_->cancel();
			}

			template < typename SetSocketOptionT >
			bool set_option(const SetSocketOptionT &option)
			{
				return impl_->set_option(option);
			}
			template < typename GetSocketOptionT >
			bool get_option(GetSocketOptionT &option)
			{
				return impl_->get_option(option)
			}
			template<typename IOControlCommandT>
			bool io_control(IOControlCommandT &control)
			{
				return impl_->io_control(control);
			}

			void bind(u_short port, const ip_address &addr)
			{
				const protocol_type &protocol = protocol_type::v4();
				impl_->bind(protocol.family(), port, addr);
			}


			// 连接远程服务
			void connect(const ip_address &addr, u_short port)
			{
				const protocol_type &protocol = protocol_type::v4();
				impl_->connect(protocol.family(), addr, port);
			}

			void dis_connect(int shut = SD_BOTH)
			{
				impl_->dis_connect(shut, true);
			}
			 
			// 异步链接
			template < typename HandlerT >
			void async_connect(const ip_address &addr, u_short port, const HandlerT &handler)
			{
				return impl_->async_connect(addr, port, handler);
			}
			

			// 异步断开链接
			template < typename HandlerT >
			void async_disconnect(const HandlerT &callback, bool reuse = true)
			{
				return impl_->async_disconnect(callback, reuse);
			}
			

			// 阻塞式发送数据直到数据发送成功或出错
			template < typename ConstBufferT >
			size_t write(const ConstBufferT &buffer)
			{
				return write(buffer, 0);
			}
			template < typename ConstBufferT >
			size_t write(const ConstBufferT &buffer, DWORD flag)
			{
				return impl_->write(buffer, flag);
			}

			// 异步发送数据
			template < typename ConstBufferT, typename HandlerT >
			void async_write(const ConstBufferT &buffer, const HandlerT &callback)
			{
				return impl_->async_write(buffer, callback);
			}

			

			// 阻塞式接收数据直到成功或出错
			template < typename MutableBufferT >
			size_t read(MutableBufferT &buffer)
			{
				return read(buffer, 0);
			}
			template < typename MutableBufferT >
			size_t read(MutableBufferT &buffer, u_long flag)
			{
				return impl_->read(buffer, flag);
			}
	

			// 异步接收数据
			template < typename MutableBufferT, typename HandlerT >
			void async_read(MutableBufferT &buffer, const HandlerT &callback)
			{
				return impl_->async_read(buffer, callback);
			}

		};
	}
}





#endif
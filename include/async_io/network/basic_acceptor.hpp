#ifndef __NETWORK_BASIC_ACCEPTOR_HPP
#define __NETWORK_BASIC_ACCEPTOR_HPP



namespace async
{
	namespace network
	{

		// -------------------------------------------------
		// class basic_acceptor_t

		template < typename ProtocolT >
		class basic_acceptor_t
		{
		public:
			typedef ProtocolT						protocol_type;
			typedef socket_handle_ptr::element_type::native_handle_type	native_handle_type;
			typedef socket_handle::dispatcher_type	dispatcher_type;	

		private:
			socket_handle_ptr impl_;

		public:
			explicit basic_acceptor_t(dispatcher_type &io)
				: impl_(make_socket(io))
			{}
			explicit basic_acceptor_t(const socket_handle_ptr &impl)
				: impl_(impl)
			{}
			basic_acceptor_t(dispatcher_type &io, const protocol_type &protocol)
				: impl_(make_socket(io, protocol.family(), protocol.type(), protocol.protocol()))
			{}
			basic_acceptor_t(dispatcher_type &io, const protocol_type &protocol, u_short port, const ip_address &addr = INADDR_ANY, bool reuseAddr = true)
				: impl_(make_socket(io, protocol.family(), protocol.type(), protocol.protocol()))
			{
				if( reuseAddr )
					impl_->set_option(reuse_addr(true));

				bind(protocol.family(), port, addr);
				listen();
			}

		public:
			native_handle_type native_handle() const
			{
				return impl_->native_handle();
			}

			void open(const protocol_type &protocol = protocol_type::v4())
			{
				if( protocol.Type() == SOCK_STREAM )
					impl_->open(protocol.Family(), protocol.Type(), protocol.Protocol());
				else
					throw std::logic_error("not Stream socket!");
			}

			bool is_open() const
			{
				return impl_->is_open();
			}

			void close()
			{
				return impl_->close();
			}

			void cancel()
			{
				return impl_->cancel();
			}

			template<typename SetSocketOptionT>
			bool set_option(const SetSocketOptionT &option)
			{
				return impl_->set_option(option);
			}
			template<typename GetSocketOptionT>
			bool get_option(GetSocketOptionT &option)
			{
				return impl_->get_option(option)
			}
			template<typename IOControlCommandT>
			bool io_control(IOControlCommandT &control)
			{
				return impl_->io_control(control);
			}

			void bind(int family, u_short port, const ip_address &addr)
			{
				// warning: only support AF_INET
				impl_->bind(family, port, addr);
			}

			void listen(int backlog = SOMAXCONN)
			{
				impl_->listen(backlog);
			}

			socket_handle_ptr accept()
			{
				return impl_->accept();
			}

			template < typename HandlerT >
			void async_accept(const socket_handle_ptr &sck, const HandlerT &callback)
			{
				return impl_->async_accept(sck, callback);
			}
		};
	}
}







#endif
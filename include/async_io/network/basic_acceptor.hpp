#ifndef __ASYNC_NETWORK_BASIC_ACCEPTOR_HPP
#define __ASYNC_NETWORK_BASIC_ACCEPTOR_HPP

#include "socket.hpp"

namespace async { namespace network {

	// -------------------------------------------------
	// class basic_acceptor_t

	template < typename ProtocolT >
	class basic_acceptor_t
	{
	public:
		typedef ProtocolT							protocol_type;
		typedef socket_handle_t::native_handle_type	native_handle_type;
		typedef socket_handle_t::dispatcher_type	dispatcher_type;	

	private:
		socket_handle_t impl_;

	public:
		explicit basic_acceptor_t(dispatcher_type &io)
			: impl_(io)
		{}
		explicit basic_acceptor_t(const socket_handle_t &sck)
			: impl_(sck)
		{}
		basic_acceptor_t(dispatcher_type &io, const protocol_type &protocol)
			: impl_(io, protocol.family(), protocol.type(), protocol.protocol())
		{}
		basic_acceptor_t(dispatcher_type &io, const protocol_type &protocol, std::uint16_t port, const ip_address &addr = INADDR_ANY, bool reuseAddr = true)
			: impl_(io, protocol.family(), protocol.type(), protocol.protocol())
		{
			if( reuseAddr )
				impl_.set_option(reuse_addr(true));

			bind(protocol.family(), port, addr);
			listen();
		}

	public:
		native_handle_type native_handle() const
		{
			return impl_.native_handle();
		}

		void open(const protocol_type &protocol = protocol_type::v4())
		{
			if( protocol.Type() == SOCK_STREAM )
				impl_.open(protocol.Family(), protocol.Type(), protocol.Protocol());
			else
				throw service::network_exception("not Stream socket!");
		}

		bool is_open() const
		{
			return impl_.is_open();
		}

		void close()
		{
			return impl_.close();
		}

		void cancel()
		{
			return impl_.cancel();
		}

		template<typename SetSocketOptionT>
		bool set_option(const SetSocketOptionT &option)
		{
			return impl_.set_option(option);
		}
		template<typename GetSocketOptionT>
		bool get_option(GetSocketOptionT &option)
		{
			return impl_.get_option(option)
		}
		template<typename IOControlCommandT>
		bool io_control(IOControlCommandT &control)
		{
			return impl_.io_control(control);
		}

		void bind(int family, u_short port, const ip_address &addr)
		{
			// warning: only support AF_INET
			impl_.bind(family, port, addr);
		}

		void listen(int backlog = SOMAXCONN)
		{
			impl_.listen(backlog);
		}

		socket_handle_t accept()
		{
			return impl_.accept();
		}

		template < typename HandlerT, typename AllocatorT >
		void async_accept(std::shared_ptr<socket_handle_t> &&sck, HandlerT &&callback, const AllocatorT &allocator)
		{
			return impl_.async_accept(std::move(sck), std::forward<HandlerT>(callback), allocator);
		}
	};
}
}







#endif
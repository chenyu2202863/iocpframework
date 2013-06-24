#ifndef __ASYNC_NETWORK_TCP_HPP
#define __ASYNC_NETWORK_TCP_HPP


#include "basic_acceptor.hpp"
#include "basic_stream_socket.hpp"
#include "socket_option.hpp"

#include "../service/read_write_buffer.hpp"
#include "../service/write.hpp"
#include "../service/read.hpp"



namespace async { namespace network {

	// --------------------------------------------------
	// class Tcp

	class tcp
	{
	public:
		typedef basic_acceptor_t<tcp>			accpetor;
		typedef basic_stream_socket_t<tcp>		socket;

	private:
		int family_;

	private:
		explicit tcp(int family)
			: family_(family)
		{}


	public:
		int type() const
		{
			return SOCK_STREAM;
		}

		int protocol() const
		{
			return IPPROTO_TCP;
		}

		int family() const
		{
			return family_;
		}

	public:
		static tcp v4()
		{
			return tcp(AF_INET);
		}

	public:
		friend bool operator==(const tcp &lhs, const tcp &rhs)
		{
			return lhs.family_ == rhs.family_;
		}
		friend bool operator!=(const tcp &lhs, const tcp &rhs)
		{
			return !(lhs == rhs);
		}
	};
}
}







#endif
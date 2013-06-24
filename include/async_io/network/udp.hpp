#ifndef __ASYNC_NETWORK_UDP_HPP
#define __ASYNC_NETWORK_UDP_HPP


#include "socket.hpp"
#include "basic_datagram_socket.hpp"
#include "socket_option.hpp"

#include "../service/write.hpp"
#include "../service/read.hpp"
#include "../service/read_write_buffer.hpp"


namespace async { namespace network {

	// --------------------------------------------------
	// class Udp

	class udp
	{
	public:
		typedef basic_datagram_socket_t<udp>		socket;


	private:
		int family_;

	private:
		explicit udp(int family)
			: family_(family)
		{}


	public:
		int type() const
		{
			return SOCK_DGRAM;
		}

		int protocol() const
		{
			return IPPROTO_UDP;
		}

		int family() const
		{
			return family_;
		}

	public:
		static udp v4()
		{
			return udp(AF_INET);
		}

	public:
		friend bool operator==(const udp &lhs, const udp &rhs)
		{
			return lhs.family_ == rhs.family_;
		}
		friend bool operator!=(const udp &lhs, const udp &rhs)
		{
			return !(lhs == rhs);
		}
	};
}
}










#endif
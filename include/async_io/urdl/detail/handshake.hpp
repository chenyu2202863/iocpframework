#ifndef __ASYNC_URDL_DETAIL_HANDSHAKE_HPP
#define __ASYNC_URDL_DETAIL_HANDSHAKE_HPP

#include <cstring>
#include <cctype>

#include "coroutine.hpp"
#include "Network/TCP.hpp"


namespace urdl 
{
	namespace detail 
	{

		inline std::error_code handshake(async::network::Tcp::Socket& /*socket*/,
			const std::string& /*host*/, std::error_code& ec)
		{
			ec = std::error_code();
			return ec;
		}

		template < typename HandlerT >
		void async_handshake(async::iocp::IODispatcher &io, const std::string& /*host*/, const HandlerT &handler)
		{
			std::error_code ec;
			io.Post(std::bind(handler, ec));
		}

	} // namespace detail
} // namespace urdl



#endif 

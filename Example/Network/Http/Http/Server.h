#ifndef __HTTP_SERVER_HPP
#define __HTTP_SERVER_HPP

#include <string>
#include <async_io\network\tcp.hpp>


#include "ConnectionMgr.h"
#include "RequestHandler.h"



namespace http
{
	
	class Server
	{
	private:
		async::iocp::io_dispatcher io_;
		async::network::tcp::accpetor acceptor_;
	
		ConnectionMgr connectionMgr_;
		RequestHandler requestHandler_;

	public:
		Server(const std::string &addr, const std::string &port, const std::string &dir);

	private:
		Server(const Server &);
		Server &operator=(const Server &);

	public:
		void Start();
		void Stop();

	private:
		void _HandleAccept(async::iocp::error_code, const async::network::socket_handle_ptr &);
		void _HandleStop();
	};
}



#endif
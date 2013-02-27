#ifndef __HTTP_SERVER_HPP
#define __HTTP_SERVER_HPP

#include <string>
#include "../../../../include/Network/TCP.hpp"


#include "ConnectionMgr.h"
#include "RequestHandler.h"



namespace http
{
	
	class Server
	{
	private:
		async::iocp::IODispatcher io_;
		async::network::Tcp::Accpetor acceptor_;
	
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
		void _HandleAccept(u_long, const async::network::SocketPtr &);
		void _HandleStop();
	};
}



#endif
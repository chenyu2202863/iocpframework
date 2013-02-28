// 02.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <network/tcp.hpp>

using namespace async;

class Server
{
	iocp::io_dispatcher &io_;
	network::tcp::accpetor acceptor_;

public:
	explicit Server(iocp::io_dispatcher &io, u_short port)
		: io_(io)
		, acceptor_(io, network::tcp::v4(), port)
	{}


	void Start()
	{
		network::socket_handle_ptr sock(network::make_socket(io_, 
			network::tcp::v4().family(), network::tcp::v4().type(), network::tcp::v4().protocol()));

		acceptor_.async_accept(sock, std::bind(
			&Server::_OnAccept, this, iocp::_Error, iocp::_Socket));
	}

	void _OnAccept(iocp::error_code err, const network::socket_handle_ptr &newSock)
	{
		
	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	iocp::io_dispatcher io(1);
	Server svr(io, 1079);
	
	system("pause");
	return 0;
}


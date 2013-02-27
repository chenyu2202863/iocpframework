// MaxConnection.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <network\tcp.hpp>

#include <atlbase.h>
#include <atlconv.h>
#include <ctime>

#include <iostream>

using namespace async;

class Server
{
	enum { MAX_CONNECTION = 1024 };

	iocp::io_dispatcher &io_;
	network::tcp::accpetor acceptor_;

	size_t connectCnt_;
	char buf_[1024];

public:
	Server(iocp::io_dispatcher &io, u_short port)
		: io_(io)
		, acceptor_(io, async::network::tcp::v4(), port)
		, connectCnt_(0)
	{
		_Start();
	}

	void _Start()
	{
		network::socket_handle_ptr sck(network::make_socket(io_, 
			network::tcp::v4().family(), network::tcp::v4().type(), network::tcp::v4().protocol()));
		acceptor_.async_accept(sck, std::bind(
			&Server::OnAccept, this, iocp::_Error, iocp::_Socket));
	}

	void OnAccept(iocp::error_code err, const network::socket_handle_ptr &newSock)
	{
		if( err != 0 )
			return;

		try
		{
			if( connectCnt_ > MAX_CONNECTION )
				newSock->shutdown(SD_BOTH);
			else
			{
				++connectCnt_;
				newSock->async_read(iocp::mutable_buffer(buf_, 1024), 
					std::bind(&Server::OnRead, this, async::iocp::_Error, async::iocp::_Size));
				_Start();
			}
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void OnRead(iocp::error_code err, u_long size)
	{
		if( err != 0 || size == 0 )
			--connectCnt_;
	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	iocp::io_dispatcher io;
	Server svr(io, 5050);

	system("pause");
	return 0;
}


// Roudtrip.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include <network/tcp.hpp>
#include <iostream>

#include <atlbase.h>
#include <atlconv.h>		// for CT2A


using namespace async;

class Server
{
	iocp::io_dispatcher &io_;
	network::tcp::accpetor acceptor_;

	char buf_[1024 * 512];

public:
	Server(iocp::io_dispatcher &io, u_short port)
		: io_(io)
		, acceptor_(io, network::tcp::v4(), port)
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
			newSock->set_option(network::no_delay(true));

			network::tcp::socket remoteSock(newSock); 
			iocp::async_read(remoteSock, iocp::buffer(buf_), iocp::transfer_at_least(sizeof(buf_)),
				std::bind(&Server::OnRead, this, remoteSock, iocp::_Error, iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void OnRead(network::tcp::socket newSock, iocp::error_code err, u_long size)
	{
		if( err != 0 )
			return;

		try
		{
			iocp::write(newSock, iocp::buffer(buf_, size), iocp::transfer_all());
			
			iocp::async_read(newSock, iocp::buffer(buf_), iocp::transfer_at_least(sizeof(buf_)),
				std::bind(&Server::OnRead, this, newSock, iocp::_Error,  iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
};



class Client
{
	iocp::io_dispatcher &io_;
	network::tcp::socket sock_;

	char buf_[1024 * 512];

public:
	Client(iocp::io_dispatcher &io, const std::string &ip, u_short port)
		: io_(io)
		, sock_(io, network::tcp::v4())
	{
		_Start(ip, port);
	}

	void _Start(const std::string &ip, u_short port)
	{
		using namespace std::tr1::placeholders;

		sock_.set_option(network::no_delay(true));
		sock_.async_connect(network::ip_address::parse(ip), port,
			std::bind(&Client::OnConnect, this, iocp::_Error));
	}

	void OnConnect(iocp::error_code err)
	{
		if( err != 0 )
			return;

		_Send();
	}

	void OnRead(iocp::error_code err, u_long size)
	{
		if( err != 0 )
			return;

		DWORD send = 0, their = 0;
		memcpy(&send, buf_, sizeof(send));
		
		//::Sleep(1000);

		DWORD back = ::GetTickCount();


		std::cout << "round trip " << back - send << std::endl;
			//<< " clock error " << their - mine << std::endl << std::endl;

		::Sleep(5000);
		_Send();
	}

	void _Send()
	{
		DWORD curTime = ::GetTickCount();

		memcpy(buf_, &curTime, sizeof(curTime));

		try
		{
			iocp::write(sock_, iocp::buffer(buf_, sizeof(buf_)), iocp::transfer_all());

			iocp::async_read(sock_, iocp::buffer(buf_), iocp::transfer_at_least(sizeof(buf_)),
				std::bind(&Client::OnRead, this, iocp::_Error, iocp::_Size));

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	iocp::io_dispatcher io(2);

	if( argc > 2 )
	{
		u_short port = static_cast<u_short>(_tstoi(argv[2]));
		if( _tcscmp(argv[1], _T("s")) == 0 )
		{
			Server svr(io, port);
			system("pause");
		}
		else
		{
			Client cli(io, (LPCSTR)CT2A(argv[1]), port);
			system("pause");
		}
	}
	else
	{
		printf("Usage:\n%s -s port\n%s ip port\n", argv[0], argv[0]);
		system("pause");
	}


	
	return 0;
}


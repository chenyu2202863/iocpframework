// Discard.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <network/tcp.hpp>

#include <atlbase.h>
#include <atlconv.h>
#include <iostream>
#include <cstdint>

using namespace async;



class DiscadCli
{
	iocp::io_dispatcher &io_;
	network::tcp::socket sock_;
	u_short port_;

	std::vector<char> buffer_;

public:
	DiscadCli(iocp::io_dispatcher &io, u_short port, size_t size)
		: io_(io)
		, sock_(io, network::tcp::v4())
		, port_(port)
	{
		buffer_.resize(size);
	}

	void Start(const std::string &ip)
	{
		try
		{
			//sock_.SetOption(RecvBufSize(512));
			//sock_.SetOption(SendBufSize(512));
			sock_.async_connect(network::ip_address::parse(ip), port_,
				std::bind(&DiscadCli::_OnConnect, this, iocp::_Error));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

private:
	void _OnConnect(u_long error)
	{
		if( error != 0 )
			return;

		try
		{
			iocp::async_write(sock_, iocp::buffer(buffer_), iocp::transfer_all(), 
				std::bind(&DiscadCli::_OnWrite, this, iocp::_Error));
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _OnWrite(u_long err)
	{
		if( err != 0 )
			return;

		try
		{
			iocp::async_read(sock_, iocp::buffer(buffer_), iocp::transfer_at_least(1), 
				std::bind(&DiscadCli::_OnRead, this, iocp::_Size, iocp::_Error));

			::Sleep(100);
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _OnRead(u_long size, u_long err)
	{
		if( err != 0 )
			return;

		try
		{
			iocp::async_write(sock_, iocp::buffer(buffer_), iocp::transfer_all(), 
				std::bind(&DiscadCli::_OnWrite, this, iocp::_Error));
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	iocp::io_dispatcher io;
	size_t msgSize = 512;
	if( argc > 3 )
		msgSize = _tstoi(argv[3]);

	std::vector<DiscadCli *> clis;
	if( argc > 2 )
	{
		size_t cnt = _tstoi(argv[2]);

		clis.reserve(cnt);
		for(size_t i = 0; i != cnt; ++i)
			clis.push_back(new DiscadCli(io, 5050, msgSize));
	}
	else
		clis.push_back(new DiscadCli(io, 5050, msgSize));

	std::string ip;
	if( argc > 1 )
		ip = CW2A(argv[1]);
	else
		ip = "127.0.0.1";

	for(size_t i = 0; i != clis.size(); ++i)
		clis[i]->Start(ip.c_str());

	system("pause");
	return 0;
}


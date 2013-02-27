// Udp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <network/udp.hpp>


using namespace async;

class Server
{
private:
	iocp::io_dispatcher& io_service_;
	network::udp::socket socket_;
	SOCKADDR_IN addr_;
	u_short port_;

	enum { max_length = 1024 };
	char data_[max_length];

public:
	Server(iocp::io_dispatcher& io, const network::ip_address &ip, u_short port)
		: io_service_(io)
		, socket_(io, network::udp::v4())
	{
		std::uninitialized_fill_n(reinterpret_cast<char *>(&addr_), sizeof(addr_), 0);
		
		addr_.sin_family = network::udp::v4().family();
		addr_.sin_addr.S_un.S_addr = ::htonl(ip);
		addr_.sin_port = ::htons(port);

		try
		{
			socket_.async_recv_from(
				iocp::buffer(data_, max_length), 
				&addr_,
				std::bind(&Server::handle_receive_from, this, iocp::_Error, iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}

	}
	~Server()
	{
		Close();
	}

	void handle_receive_from(iocp::error_code error, size_t bytes_recvd)
	{
		if( error != 0 )
			return;

		try
		{
			if ( bytes_recvd > 0 )
			{
				socket_.async_send_to(
					iocp::buffer(data_, bytes_recvd), &addr_,
					std::bind(&Server::handle_send_to, this, iocp::_Error, iocp::_Size));
			}
			else
			{
				socket_.async_recv_from(
					iocp::buffer(data_, max_length), &addr_,
					std::bind(&Server::handle_receive_from, this, iocp::_Error, iocp::_Size));
			}
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void handle_send_to(iocp::error_code error, size_t bytes_sent)
	{
		try
		{
			socket_.async_recv_from(
				iocp::buffer(data_, max_length), &addr_,
				std::bind(&Server::handle_receive_from, this, iocp::_Error, iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Close()
	{
		socket_.cancel();
		socket_.close();
	}

};

int main(int argc, char* argv[])
{
	try
	{
		iocp::io_dispatcher io;

		using namespace std; // For atoi.
		Server s(io, network::ip_address::parse("127.0.0.1"), 5050);

		system("pause");
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	system("pause");
	return 0;
}


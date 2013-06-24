#ifndef __CLIENT_IMPL_HPP
#define __CLIENT_IMPL_HPP

#include <iostream>
#include <network/tcp.hpp>
#include <cstdint>

using namespace async;


class Client
{
private:
	iocp::io_dispatcher &io_;
	network::tcp::socket socket_;
	std::vector<char> buf_;

public:
	Client(iocp::io_dispatcher &io, const std::string &ip, u_short port)
		: io_(io)
		, socket_(io, network::tcp::v4())
	{
		const size_t len = 1024;
		buf_.resize(sizeof(std::uint32_t) + 128);
		struct
		{
			std::uint32_t len;
			std::uint16_t cmd;
		}msg = { 128, 0x01 };
		memcpy(buf_.data(), &msg, sizeof(msg));
		memset(&buf_[sizeof(msg)], 10, 128 - sizeof(std::uint32_t));

		try
		{
			socket_.async_connect(network::ip_address::parse(ip), port, 
				std::bind(&Client::_OnConnect, this, iocp::_Error));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}


private:
	void _OnConnect(iocp::error_code error)
	{
		if( error != 0 )
			return;

	
		try
		{
			iocp::async_write(socket_, iocp::buffer(buf_), iocp::transfer_all(), 
				std::bind(&Client::_HandleWrite, this, iocp::_Error, iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _HandleRead(iocp::error_code error, u_long bytes)
	{
		try
		{
			if( bytes == 0 || error != 0 )
			{
				socket_.async_disconnect(std::bind(&Client::_DisConnect, this));
				return;
			}


			::Sleep(1000);

			iocp::async_write(socket_, iocp::buffer(buf_), iocp::transfer_all(), 
				std::bind(&Client::_HandleWrite, this, iocp::_Error, iocp::_Size));
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;

			socket_.async_disconnect(std::bind(&Client::_DisConnect, this));
		}

	}

	void _HandleWrite(iocp::error_code error, u_long bytes)
	{
		try
		{		
			if( bytes == 0 || error != 0 )
			{
				socket_.async_disconnect(std::bind(&Client::_DisConnect, this));
				return;
			}

			iocp::async_read(socket_, iocp::buffer(buf_), iocp::transfer_all(),
				std::bind(&Client::_HandleRead, this, iocp::_Error, iocp::_Size));

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;

			socket_.async_disconnect(std::bind(&Client::_DisConnect, this));
		}
	}

	void _DisConnect()
	{
		socket_.close();
	}
};



#endif
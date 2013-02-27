#ifndef __CLIENT_IMPL_HPP
#define __CLIENT_IMPL_HPP


#include "../../../../include/Network/TCP.hpp"
#include "../CRC32.hpp"

using namespace async::network;



class Client
{
private:
	IODispatcher &io_;
	Tcp::Socket socket_;
	std::vector<char> buf_;

public:
	Client(IODispatcher &io, const std::string &ip, u_short port)
		: io_(io)
		, socket_(io, Tcp::V4())
	{
		const size_t len = 1024;
		buf_.resize(len);
		std::fill(buf_.begin(), buf_.end(), 100);

		size_t crc = algorithm::crc::cac_crc32(&buf_[sizeof(size_t)], len - sizeof(size_t));
		const char *beg = reinterpret_cast<char *>(&crc);
		const char *end = beg + sizeof(size_t);
		std::copy(beg, end, buf_.begin());
		

		try
		{
			socket_.AsyncConnect(IPAddress::Parse(ip), port, 
				std::tr1::bind(&Client::_OnConnect, this, _Size, _Error));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}


private:
	void _OnConnect(u_long bytes, u_long error)
	{
		if( error != 0 )
			return;

		//static char msg[] = "I am a new client";

		try
		{
			AsyncWrite(socket_, Buffer(buf_), TransferAll(), 
				std::tr1::bind(&Client::_HandleWrite, this, _Size, _Error));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _HandleRead(u_long bytes, u_long error)
	{
		try
		{
			if( bytes == 0 )
			{
				socket_.AsyncDisconnect(std::tr1::bind(&Client::_DisConnect, this));
				return;
			}

			//std::cout.write(buf_.data(), bytes) << std::endl;

			::Sleep(2000);

			AsyncWrite(socket_, Buffer(buf_), TransferAll(), 
				std::tr1::bind(&Client::_HandleWrite, this, _Size, _Error));
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}

	}

	void _HandleWrite(u_long bytes, u_long error)
	{
		try
		{		
			if( bytes == 0 )
			{
				socket_.AsyncDisconnect(std::tr1::bind(&Client::_DisConnect, this));
				return;
			}

			AsyncRead(socket_, Buffer(buf_), TransferAll(),
				std::tr1::bind(&Client::_HandleRead, this, _Size, _Error));

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _DisConnect()
	{
		socket_.Close();
	}
};



#endif
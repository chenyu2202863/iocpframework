#ifndef __TUNNEL_HPP
#define __TUNNEL_HPP

#include <iostream>
#include <iocp/dispatcher.hpp>
#include <network/tcp.hpp>


using namespace async;	// 别鄙视，就图省事儿

class Tunnel
	: public std::enable_shared_from_this<Tunnel>
{
	network::tcp::socket sock_;
	network::tcp::socket connect_;

	char buffer_[1024];

public:
	Tunnel(iocp::io_dispatcher &io, const network::socket_handle_ptr &sock)
		: sock_(sock)
		, connect_(io, network::tcp::v4())
	{
	}
	~Tunnel()
	{

	}

public:
	void Start(const std::string &svrIP, u_short svrPort)
	{
		sock_.set_option(network::no_delay(true));
			
		try
		{
			connect_.async_connect(network::ip_address::parse(svrIP), svrPort, 
				std::bind(&Tunnel::_OnSvrConnect, shared_from_this(), iocp::_Error));

			sock_.async_read(iocp::buffer(buffer_), 
				std::bind(&Tunnel::_OnCliRead, shared_from_this(), iocp::_Error, iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Stop()
	{
		_DisConnect();
	}


private:
	void _OnSvrConnect(iocp::error_code error)
	{
		if( error == 0 )
			return;
		else
			_DisConnect();
	}

	void _OnCliRead(iocp::error_code error, u_long bytes)
	{
		try
		{
			if( bytes == 0 || error != 0 )
			{
				_DisConnect();
				return;
			}


			iocp::async_write(connect_, iocp::buffer(buffer_, bytes), iocp::transfer_all(), 
				std::bind(&Tunnel::_OnSvrWrite, shared_from_this(), iocp::_Error, iocp::_Size));
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			_DisConnect();
		}

	}

	void _OnSvrRead(iocp::error_code error, u_long bytes)
	{
		try
		{
			if( bytes == 0 || error != 0 )
			{
				_DisConnect();
				return;
			}


			iocp::async_write(sock_, iocp::buffer(buffer_, bytes), iocp::transfer_all(), 
				std::bind(&Tunnel::_OnCliWrite, shared_from_this(), iocp::_Error, iocp::_Size));
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			_DisConnect();
		}

	}


	void _OnSvrWrite(u_long error, u_long bytes)
	{
		try
		{		
			if( bytes == 0 || error != 0 )
			{
				_DisConnect();
				return;
			}

			connect_.async_read(iocp::buffer(buffer_), 
				std::tr1::bind(&Tunnel::_OnSvrRead, shared_from_this(), iocp::_Error, iocp::_Size));

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			_DisConnect();
		}
	}

	void _OnCliWrite(u_long error, u_long bytes)
	{
		try
		{		
			if( bytes == 0 || error != 0 )
			{
				_DisConnect();
				return;
			}

			sock_.async_read(iocp::buffer(buffer_), 
				std::bind(&Tunnel::_OnCliRead, shared_from_this(), iocp::_Error, iocp::_Size));

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			_DisConnect();
		}
	}

	void _DisConnect()
	{
		sock_.close();
	}

};

typedef std::shared_ptr<Tunnel> TunnelPtr;


class Server
{
	iocp::io_dispatcher &io_;
	network::tcp::accpetor acceptor_;

	const std::string connectIP_;
	u_short connectPort_;

public:
	Server(iocp::io_dispatcher &io, u_short listenPort, const std::string &connectIP, u_short connectPort)
		: io_(io)
		, acceptor_(io, network::tcp::v4(), listenPort)
		, connectIP_(connectIP)
		, connectPort_(connectPort)
	{
		
	}

public:
	void Start()
	{
		_StartAccept();
	}

	void Stop()
	{
		_StopServer();
	}

private:
	void _StartAccept()
	{		
		try
		{
			network::socket_handle_ptr sck(network::make_socket(io_, 
				network::tcp::v4().family(), network::tcp::v4().type(), network::tcp::v4().protocol()));
			acceptor_.async_accept(sck,
				std::bind(&Server::_OnAccept, this, std::placeholders::_1, std::placeholders::_2));
		} 
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _StopServer()
	{
		acceptor_.close();
	}

private:
	void _OnAccept(iocp::error_code error, const network::socket_handle_ptr &new_sock)
	{
		if( error != 0 )
			return;

		try
		{
			TunnelPtr tunnel(new Tunnel(io_, new_sock));
			tunnel->Start(connectIP_, connectPort_);

			_StartAccept();
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
};





#endif
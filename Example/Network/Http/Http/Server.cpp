#include "stdafx.h"
#include "Server.h"

#include <iostream>
#include "Connection.h"


namespace http
{

	Server::Server(const std::string &addr, const std::string &port, const std::string &dir)
		: io_()
		, acceptor_(io_, async::network::tcp::v4(), ::atoi(port.c_str()))
		, connectionMgr_()
		, requestHandler_(dir)
	{
		
	}

	void Server::Start()
	{
		async::network::socket_handle_ptr sck(async::network::make_socket(io_, 
			async::network::tcp::v4().family(), async::network::tcp::v4().type(), async::network::tcp::v4().protocol()));
		
		acceptor_.async_accept(sck, 
			std::bind(&Server::_HandleAccept, this, async::iocp::_Error, async::iocp::_Socket));
	}

	void Server::Stop()
	{
		io_.post(std::bind(&Server::_HandleStop, this));
	}

	void Server::_HandleAccept(async::iocp::error_code error, const async::network::socket_handle_ptr &remoteSocket)
	{
		if( error != 0 )
		{
			// ... ´íÎó
			return;
		}

		try
		{
			ConnectionPtr connection(new Connection(remoteSocket, connectionMgr_, requestHandler_));
			connectionMgr_.Start(connection);

			Start();
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Server::_HandleStop()
	{
		acceptor_.close();
		connectionMgr_.StopAll();
	}
}
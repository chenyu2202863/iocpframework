#include "stdafx.h"
#include "Server.h"

#include "Connection.h"
#include "../../../../include/IOCP/AsyncResult.hpp"


namespace http
{

	Server::Server(const std::string &addr, const std::string &port, const std::string &dir)
		: io_()
		, acceptor_(io_, async::network::Tcp::V4(), ::atoi(port.c_str()))
		, connectionMgr_()
		, requestHandler_(dir)
	{
		
	}

	void Server::Start()
	{
		acceptor_.AsyncAccept(0, 
			std::tr1::bind(&Server::_HandleAccept, this, async::iocp::_Error, async::iocp::_Socket));
	}

	void Server::Stop()
	{
		io_.Post(std::tr1::bind(&Server::_HandleStop, this));
	}

	void Server::_HandleAccept(u_long error, const async::network::SocketPtr &remoteSocket)
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

			acceptor_.AsyncAccept(0, 
				std::tr1::bind(&Server::_HandleAccept, this, async::iocp::_Error, async::iocp::_Socket));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Server::_HandleStop()
	{
		acceptor_.Close();
		connectionMgr_.StopAll();
	}
}
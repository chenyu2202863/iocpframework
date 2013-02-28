#ifndef __HTTP_CONNECTION_HPP
#define __HTTP_CONNECTION_HPP

#include <array>
#include <memory>

#include <async_io/network/tcp.hpp>

#include "Reply.h"
#include "Request.h"
#include "RequestHandler.h"
#include "RequestParser.h"



namespace http
{

	class ConnectionMgr;

	class Connection
		: public std::tr1::enable_shared_from_this<Connection>
	{
	private:
		async::network::tcp::socket socket_;
		ConnectionMgr &connectionMgr_;
		RequestHandler &requestHandler_;

		std::tr1::array<char, 8192> buffer_;
		async::iocp::auto_buffer socketBuffer_;
		Request request_;
		RequestParser requestParser_;
		Reply reply_;

	public:
		Connection(const async::network::socket_handle_ptr &sock, ConnectionMgr &mgr, RequestHandler &handler);

	private:
		Connection(const Connection &);
		Connection &operator=(const Connection &);

	public:
		async::network::tcp::socket &Socket();

		void Start();
		void Stop();

	private:
		void _CopyBuffer(const std::vector<async::iocp::const_buffer> &buf);
		void _HandleRead(async::iocp::error_code, u_long);
		void _HandleWrite(async::iocp::error_code, u_long);
	};


	typedef std::shared_ptr<Connection>	ConnectionPtr;
}

#endif
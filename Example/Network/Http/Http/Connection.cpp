#include "stdafx.h"
#include "connection.h"

#include <iostream>
#include <vector>
#include "ConnectionMgr.h"
#include "RequestHandler.h"



namespace http
{
	
	Connection::Connection(const async::network::socket_handle_ptr &sock, ConnectionMgr &mgr, RequestHandler &handler)
		: socket_(sock)
		, connectionMgr_(mgr)
		, requestHandler_(handler)
		, socketBuffer_()
	{}


	async::network::tcp::socket &Connection::Socket()
	{
		return socket_;
	}

	void Connection::Start()
	{
		socket_.async_read(async::iocp::buffer(buffer_), 
			std::bind(&Connection::_HandleRead, shared_from_this(), async::iocp::_Error, async::iocp::_Size));
	}

	void Connection::Stop()
	{
		socket_.close();
	}
	

	void Connection::_CopyBuffer(const std::vector<async::iocp::const_buffer> &buf)
	{
		size_t total = 0;
		for(size_t i = 0; i != buf.size(); ++i)
		{
			total += buf[i].size();
		}
		socketBuffer_.resize(total);

		size_t index = 0;
		for(size_t i =0; i != buf.size(); ++i)
		{
			std::copy(buf[i].begin(), buf[i].end(), 
				stdext::make_checked_array_iterator(socketBuffer_.data(index), socketBuffer_.size() - index));
			index += buf[i].size();
		}
	}

	void Connection::_HandleRead(async::iocp::error_code error, u_long bytes)
	{
		if( error == ERROR_OPERATION_ABORTED || bytes == 0 )
		{
			connectionMgr_.Stop(shared_from_this());
			return;
		}
		else if( !error )
		{
			try
			{
				ParseRet result, ignore;
				std::tie(result, ignore) = requestParser_.Parse(request_, buffer_.data(), buffer_.data() + bytes);

				if( result == TRUE_VALUE )
				{
					requestHandler_.HandleRequest(request_, reply_);

					_CopyBuffer(reply_.ToBuffers());
					async::iocp::async_write(socket_, async::iocp::buffer(socketBuffer_.data(), socketBuffer_.size()),
						std::bind(&Connection::_HandleWrite, shared_from_this(), async::iocp::_Error, async::iocp::_Size));
				}	
				else if( result == FALSE_VALUE )
				{
					reply_ = Reply::StockReply(Reply::bad_request);

					_CopyBuffer(reply_.ToBuffers());
					async::iocp::async_write(socket_, async::iocp::buffer(socketBuffer_.data(), socketBuffer_.size()),
						std::bind(&Connection::_HandleWrite, shared_from_this(), async::iocp::_Error, async::iocp::_Size));
				}
				else	// ParseRet::INDETERMINATE
				{
					socket_.async_read(async::iocp::buffer(buffer_), 
						std::bind(&Connection::_HandleRead, shared_from_this(), async::iocp::_Error, async::iocp::_Size));
				}
			}
			catch(std::exception &e)
			{
				std::cerr << e.what() << std::endl;

				connectionMgr_.Stop(shared_from_this());
			}
		}
	}


	void Connection::_HandleWrite(async::iocp::error_code error, u_long bytes)
	{
		// нч╢М
		if( error == 0 )
		{
			socket_.shutdown(SD_BOTH);
			//socket_.AsyncDisconnect(NULL);
		}
		
		//if( error != ERROR_OPERATION_ABORTED )
		{
			connectionMgr_.Stop(shared_from_this());
		}
	}
}
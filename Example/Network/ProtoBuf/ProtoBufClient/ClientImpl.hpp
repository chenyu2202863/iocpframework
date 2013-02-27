#ifndef __CLIENT_IMPL_HPP
#define __CLIENT_IMPL_HPP
#include "test.pb.h"

#include "../../../../include/Network/TCP.hpp"
#include "../ProtoBuf/Dispatcher.hpp"
#include "../ProtoBuf/Codec.hpp"


using namespace async;
using namespace std::tr1::placeholders;

typedef std::tr1::shared_ptr<muduo::Empty> EmptyPtr;
typedef std::tr1::shared_ptr<muduo::Answer> AnswerPtr;


class Client
{
private:
	iocp::IODispatcher &io_;
	network::Tcp::Socket socket_;
	std::tr1::array<char, 4096>	buf_;

	dispatch::ProtobufDispatcher<Client> dispatcher_;
	ProtobufCodec<Client> codec_;

public:
	Client(iocp::IODispatcher &io, const std::string &ip, u_short port)
		: io_(io)
		, socket_(io, network::Tcp::V4())
		, dispatcher_(std::tr1::bind(&Client::_OnUnknownMessage, this, _1, _2))
		, codec_(std::tr1::bind(&dispatch::ProtobufDispatcher<Client>::OnProtobufMessage, &dispatcher_, _1, _2))
	{
		using namespace std::tr1::placeholders;
		dispatcher_.RegisterMessageCallback<muduo::Answer>(std::tr1::bind(&Client::_OnAnswer, this, _1, _2));
		dispatcher_.RegisterMessageCallback<muduo::Empty>(std::tr1::bind(&Client::_OnEmpty, this, _1, _2));


		try
		{
			socket_.AsyncConnect(network::IPAddress::Parse(ip), port, 
				std::tr1::bind(&Client::_OnConnect, this, iocp::_Error, iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Close()
	{
		socket_.Close();
	}

	template < typename MsgT >
	void Send(const MsgT &msg)
	{
		try
		{
			codec_.Send(socket_, msg, 
				std::tr1::bind(&Client::_HandleWrite, this, iocp::_Error, iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

private:
	void _OnConnect(u_long error, u_long bytes)
	{
		if( error != 0 )
			return;
	}

	void _HandleRead(u_long error, u_long bytes)
	{
		try
		{
			if( bytes == 0 )
			{
				socket_.AsyncDisconnect(std::tr1::bind(&Client::_DisConnect, this));
				return;
			}

			std::cout.write(buf_.data(), bytes) << std::endl;
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}

	}

	void _HandleWrite(u_long error, u_long bytes)
	{
		try
		{		
			if( error != 0 || bytes == 0 )
			{
				socket_.AsyncDisconnect(std::tr1::bind(&Client::_DisConnect, this));
				return;
			}

			iocp::AsyncRead(socket_, iocp::Buffer(buf_), iocp::TransferAtLeast(1),
				std::tr1::bind(&Client::_HandleRead, this, iocp::_Error, iocp::_Size));

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

	void _OnUnknownMessage(Client &, const MessagePtr& message)
	{
		std::cout << "onUnknownMessage: " << message->GetTypeName();
	}

	void _OnAnswer(Client &, const AnswerPtr& message)
	{
		std::cout << "onAnswer:\n" << message->GetTypeName() << message->DebugString();
	}

	void _OnEmpty(Client &, const EmptyPtr& message)
	{
		std::cout << "onEmpty: " << message->GetTypeName();
	}
};



#endif
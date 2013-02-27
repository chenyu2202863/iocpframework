#ifndef __SERVER_HPP
#define __SERVER_HPP

#include "../../../../include/IOCP/Dispatcher.hpp"
#include "../../../../include/Network/TCP.hpp"

#include "Dispatcher.hpp"
#include "test.pb.h"

using namespace async;


class Session;
typedef std::tr1::shared_ptr<Session> SessionPtr;

class Session
	: public std::tr1::enable_shared_from_this<Session>
{
public:
	typedef std::tr1::function<void(Session &, const iocp::ConstBuffer &)> OnReadComplate;

private:
	network::Tcp::Socket socket_;
	std::tr1::array<char, 1024> buf_;

	iocp::CallbackType readHeader_;
	iocp::CallbackType readContent_;
	iocp::CallbackType writeComplate_;

	ProtobufCodec<Session> &codec_;

	OnReadComplate onReadComplate_;

public:
	Session(const network::SocketPtr &sock, ProtobufCodec<Session> &codec, const OnReadComplate &onReadComplate)
		: socket_(sock)
		, codec_(codec)
		, onReadComplate_(onReadComplate)
	{
		//socket_.IOControl(network::NonBlockingIO(true));
	}
	~Session()
	{
		//Stop();
	}


public:
	network::Tcp::Socket& GetSocket()
	{
		return socket_;
	}

	void Start()
	{
		try
		{		
			readHeader_	= std::tr1::bind(&Session::_HandleHeader, shared_from_this(), iocp::_Size, iocp::_Error);
			readContent_ = std::tr1::bind(&Session::_HandleContent, shared_from_this(), iocp:: _Size, iocp::_Error);
			writeComplate_ = std::tr1::bind(&Session::_HandleWrite, shared_from_this(), iocp::_Size, iocp::_Error);

			typedef int int32;
			iocp::AsyncRead(socket_, iocp::Buffer(buf_, sizeof(int32)), iocp::TransferAtLeast(sizeof(int32)), readHeader_);

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Close()
	{
		socket_.Close();

		readHeader_		= 0;
		readContent_	= 0;
		writeComplate_	= 0;
	}

	template < typename MsgT >
	void Send(const MsgT &msg)
	{
		try
		{
			codec_.Send(socket_, msg, writeComplate_);
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

private:
	void _HandleHeader(u_long bytes, u_long error)
	{
		try
		{
			if( error != 0 || bytes == 0 )
			{
				_DisConnect();
				return;
			}

			typedef int int32;
			const int32 &len = *(reinterpret_cast<int32 *>(buf_.data()));
			iocp::AsyncRead(socket_, iocp::Buffer(buf_.data() + bytes, buf_.size()), iocp::TransferAtLeast(len), readContent_);
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _HandleContent(u_long bytes, u_long error)
	{
		try
		{
			if( error != 0 || bytes == 0 )
			{
				_DisConnect();
				return;
			}

			typedef int int32;
			onReadComplate_(*this, std::tr1::cref(iocp::ConstBuffer(buf_.data(), bytes + sizeof(int32))));
			
			AsyncRead(socket_, iocp::Buffer(buf_, sizeof(int32)), iocp::TransferAtLeast(sizeof(int32)), readHeader_);
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
			if( error != 0 || bytes == 0 )
			{
				_DisConnect();
				return;
			}
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _DisConnect()
	{
		Close();
	}
};





// 定制自己的工厂
namespace async
{
	namespace iocp
	{
		template<>
		struct ObjectFactory< Session >
		{
			typedef async::memory::FixedMemoryPool<true, sizeof(Session)>	PoolType;
			typedef ObjectPool< PoolType >									ObjectPoolType;
		};
	}
}


// to do shared_ptr allocator

template < typename CodecT, typename HandlerT >
inline SessionPtr CreateSession(const network::SocketPtr &socket, CodecT &codec, const HandlerT &handler)
{
	return SessionPtr(iocp::ObjectAllocate<Session>(socket, codec, handler), &iocp::ObjectDeallocate<Session>);
}

using namespace std::tr1::placeholders;


class Server
{
private:
	iocp::IODispatcher &io_;
	network::Tcp::Accpetor acceptor_;
	dispatch::ProtobufDispatcher<Session> dispatcher_;
	ProtobufCodec<Session> codec_;

public:
	Server(iocp::IODispatcher &io, u_short port)
		: io_(io)
		, acceptor_(io_, network::Tcp::V4(), port, INADDR_ANY)
		, dispatcher_(std::tr1::bind(&Server::_OnDefaultMessage, this, _1, _2))
		, codec_(std::tr1::bind(&dispatch::ProtobufDispatcher<Session>::OnProtobufMessage, &dispatcher_, _1, _2))
	{}

	~Server()
	{
		//_StopServer();
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

	template < typename T, typename HandlerT >
	void Register(const HandlerT &handler)
	{
		dispatcher_.RegisterMessageCallback<T>(handler);
	}

private:
	void _StartAccept()
	{		
		try
		{
			acceptor_.AsyncAccept(0, 
				std::tr1::bind(&Server::_OnAccept, this, iocp::_Error, iocp::_Socket));
		} 
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _StopServer()
	{
		acceptor_.Close();
	}



private:
	void _OnAccept(u_long error, const network::SocketPtr &acceptSocket)
	{
		if( error != 0 )
			return;

		try
		{
			static Session::OnReadComplate readComplate = std::tr1::bind(&Server::_OnMessage,
				this, iocp::_Error, iocp::_Socket);
			SessionPtr session(CreateSession(acceptSocket, codec_, readComplate));
			session->Start();

			_StartAccept();
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _OnMessage(Session &session, const iocp::ConstBuffer &buffer)
	{
		codec_.OnMessage(session, buffer);
	}

	void _OnDefaultMessage(Session &session, const MessagePtr& message)
	{
		
	}

};


#endif

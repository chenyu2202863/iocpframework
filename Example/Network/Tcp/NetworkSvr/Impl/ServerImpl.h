#ifndef __SERVICE_HPP
#define __SERVICE_HPP

#include <array>

#include <network/tcp.hpp>
#include <../memory_pool/fixed_memory_pool.hpp>


using namespace async;

volatile long g_ClientNum = 0;	// atomic


class Session
	: public std::enable_shared_from_this<Session>
{
private:
	network::tcp::socket socket_;
	std::array<char, 1024> buf_;

	iocp::rw_callback_type readCallback_;
	iocp::rw_callback_type writeCallback_;

public:
	explicit Session(const network::socket_handle_ptr &sock)
		: socket_(sock)
	{
		socket_.set_option(network::no_delay(true));
		::InterlockedIncrement(&g_ClientNum);
	}
	~Session()
	{
		::InterlockedDecrement(&g_ClientNum);
	}


public:
	network::tcp::socket& GetSocket()
	{
		return socket_;
	}

	void Start()
	{
		try
		{		
			readCallback_	= std::bind(&Session::_HandleRead, shared_from_this(), iocp::_Error, iocp::_Size);
			writeCallback_	= std::bind(&Session::_HandleWrite, shared_from_this(), iocp::_Error, iocp::_Size);

			iocp::async_read(socket_, iocp::buffer(buf_), iocp::transfer_all(), readCallback_);
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			Stop();
		}
	}

	void Stop()
	{
		socket_.close();

		readCallback_	= nullptr;
		writeCallback_	= nullptr;
	}

private:
	void _HandleRead(iocp::error_code error, u_long bytes)
	{
		try
		{
			if( bytes == 0 )
			{
				Stop();
				return;
			}

			iocp::async_write(socket_, iocp::buffer(buf_), iocp::transfer_all(), writeCallback_);
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			Stop();
		}
	}

	void _HandleWrite(iocp::error_code error, u_long bytes)
	{
		try
		{		
			iocp::async_read(socket_, iocp::buffer(buf_), iocp::transfer_all(), readCallback_);
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			Stop();
		}
	}

	void _DisConnect()
	{
		Stop();
	}
};

typedef std::shared_ptr<Session> SessionPtr;




// 定制自己的工厂
namespace async
{
	namespace iocp
	{
		template<>
		struct object_factory_t< Session >
		{
			typedef memory_pool::fixed_memory_pool_t<true, sizeof(Session)>		PoolType;
			typedef object_pool_t< PoolType >								ObjectPoolType;
		};
	}
}



inline SessionPtr CreateSession(const network::socket_handle_ptr &socket)
{
	return SessionPtr(iocp::object_allocate<Session>(socket), &iocp::object_deallocate<Session>);
	//return SessionPtr(new Session(io, socket));
}



class Server
{
private:
	iocp::io_dispatcher &io_;
	network::tcp::accpetor acceptor_;

public:
	Server(iocp::io_dispatcher &io, short port)
		: io_(io)
		, acceptor_(io_, network::tcp::v4(), port)
	{

	}

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

private:
	void _StartAccept()
	{		
		try
		{
			network::socket_handle_ptr sck(network::make_socket(io_, 
				network::tcp::v4().family(), network::tcp::v4().type(), network::tcp::v4().protocol()));
			
			acceptor_.async_accept(sck, 
				std::bind(&Server::_OnAccept, this, iocp::_Error, iocp::_Socket));
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
	void _OnAccept(iocp::error_code error, const network::socket_handle_ptr &acceptSocket)
	{
		if( error != 0 )
			return;

		try
		{
			SessionPtr session(CreateSession(acceptSocket));
			session->Start();
	
			_StartAccept();
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
};



#endif